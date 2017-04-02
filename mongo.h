#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/sha.h>
using namespace std;


class MONGO { 
	private:
	mongoc_client_t      *client;
	mongoc_database_t    *database;
	mongoc_collection_t  *teams;
	bson_t               *command,
				reply,
				*_insert,
				*_query,
				*_update;
	bson_error_t         error;
	char                 *str;
	bool                 retval;
	int count;

	public:
	/**
	*
	* 	Initialise connection to mongo database and get required pointers.  Deletes old game data from previous competitions.  
	*
	*/
	void init() {
		mongoc_init ();
		client = mongoc_client_new ("mongodb://terminator:hrsyqrzjcxfuuagwxrheksvyzcsfmqjg@localhost:27017");
		database = mongoc_client_get_database (client, "go_server");
		teams = mongoc_client_get_collection (client, "go_server", "teams");
		delete_collection("live_teams"); delete_collection("match_status");
		count = 0;
	}

	void mongo_reset() {
		/*mongoc_client_destroy(client);
		mongoc_cleanup();

		mongoc_init ();
		client = mongoc_client_new ("mongodb://terminator:hrsyqrzjcxfuuagwxrheksvyzcsfmqjg@localhost:27017");
		database = mongoc_client_get_database (client, "go_server");
		teams = mongoc_client_get_collection (client, "go_server", "teams");*/
	}
		

	/**
	*
	*	Authenticates player.  Checks hash of players password and players username against the mongo database.  SHA256 is used to find password hash. 
	*	@param team_name is the players username.  
	*	@param client_pass clients password in plaintext
	*	@return true if authentication is successfull, false on failure. 
	*
	*/
	bool auth_team(char* team_name, string client_pass) {
		mongoc_cursor_t *cursor;
		bson_error_t error;
		const bson_t *doc;
		char *str;
		bson_t *query;

		query = bson_new();
		BSON_APPEND_UTF8 (query, "team_name", team_name);	

		cursor = mongoc_collection_find (teams, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

		char search[] = "\"password\" : \"";
		char *password = (char*) malloc( BUFSIZ * sizeof(char));
		int j = 0; bool pass = false; string real;

		while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		for(int i = 0; i < strlen(str); i++) {
			if(search[j] == str[i] && !pass) {
				if(j+1 == strlen(search)) {
					pass = true;
					j = 0; continue;
				}
				j++;
			} else if(!pass) j = 0;

			if(pass) {
				if(str[i] == '"') { 
					password[j] = '\0';
					break;
				}
				password[j] = str[i];
				j++;
			}
		}
		real = sha256(client_pass);
		bson_free (str);
		}

		if (mongoc_cursor_error (cursor, &error)) {
			fprintf (stderr, "An error occurred (auth team): %s\n", error.message);
		}

		   mongoc_cursor_destroy (cursor);
		   bson_destroy (query);

		if(real==password) return true;
		else return false;
	}
			
	/**
	*
	*	Insert data into the mongo.  Data must be a string.
	*	@param key references data to be inserted
	*	@param data string of data to be inserted
	*	@param *collection pointer to collection to be accessed.
	*
	*/
	void insert(char* key, char* data, mongoc_collection_t *collection) {
		_insert = BCON_NEW (key, BCON_UTF8 (data));
		
		if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, _insert, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}
	}

	/**
	*
	*	Record a team in the mongo database which will be live. 
	*	@param team_name is the name of the team about to compete in the competition.
	*
	*/
	void record_team(char* team_name) {
		mongoc_collection_t *live_teams = mongoc_client_get_collection (client, "go_server", "live_teams");
		insert("team", team_name, live_teams);	
	}

	/**
	*
	*	Before the competition starts, for a given match fixture record the status of that match which is "not started".
	*	@param player1 string of player 1's name.
	*	@param player2 string of player 2's name.
	*
	*/
	void match_status_setup(char* player1, char* player2) {
		mongoc_collection_t *match_status = mongoc_client_get_collection (client, "go_server", "match_status");
		insert(player1, player2, match_status);
		update(player1, player2, "player #1", player1, match_status);
		update(player1, player2, "player #2", player2, match_status);
		update(player1, player2, "status", "NOT STARTED", match_status);
	}	

	/**
	*
	*	Update function which can update data in mongo database of the form key:data.
	*	@param key1 the key of the data to be updated in given collection.
	*	@param key2 the data which corresonds to key1 already in the database before update.
	*	@param key_update is the data to replace the old data with.
	*	@param collection pointer to collection to be updated.
	*
	*/
	void update(char* key1, char* key2,  char* key_update, char* __update, mongoc_collection_t *collection) {
		_query = BCON_NEW ( key1 , BCON_UTF8( key2 ) ) ;
		_update = BCON_NEW ( "$set", "{", key1 , BCON_UTF8 ( key2 ), key_update , BCON_UTF8 ( __update ), "}");
		
		if(!mongoc_collection_update (collection, MONGOC_UPDATE_NONE, _query, _update, NULL, &error)) {
			printf("UPDATE: %s : reattempting\n", error.message);
			sleep(1);
			mongo_reset();
			update(key1, key2, key_update, __update, collection);
		}
	}

	/**
	*
	*	Deletes a collection.
	*	@param _collection collection to be deleted.
	*
	*/
	void delete_collection ( char *_collection ) {
		_query = bson_new();		const bson_t *doc;
		mongoc_collection_t *collection = mongoc_client_get_collection ( client, "go_server", _collection );

		mongoc_cursor_t *cursor;
		cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, _query, NULL, NULL);
		while (mongoc_cursor_next (cursor, &doc)) {
			if(!mongoc_collection_remove ( collection, MONGOC_REMOVE_SINGLE_REMOVE, doc, NULL, &error ) ) {
				fprintf ( stderr, "Delete failed: %s\n", error.message ) ;
			}
		}
	}

	/**	Update match status of a given match.
	*
	*	A match can be in one of three statuses, in progress, game finished, or not started.  Function updates status according to @param status.	
	*	@param player1 string of player 1's name.
	*	@param player2 string of player 2's name.
	*	@param status char represents status given match should be.  'P' for in progress, 'F' for finished, 'S' for not started.
	*
	*/
	void match_status(char *player1, char* player2, char status ) {
		mongoc_collection_t *match_status = mongoc_client_get_collection (client, "go_server", "match_status");
		if(status == 'P') update(player1, player2, "status", "IN PROGRESS", match_status);				
		else if(status == 'F') update(player1, player2, "status", "GAME FINISHED", match_status);
		else if(status == 'S') update(player1, player2, "status", "NOT STARTED", match_status);
	}

	/**
	*
	*	Record game board state of given match in mongo.
	*	@param player1 string of first player in match.
	*	@param player2 string of second player in match.
	*	@json Game board state in json format.
	*
	*/
	void record_match(char *player1, char* player2, unsigned char* json, char* game_board) {
		char match[BUFSIZ];
		strcpy(match, player1); strcat(match, " VS "); strcat(match, player2);
		mongoc_collection_t *record_match = mongoc_client_get_collection (client, "go_comp_results", match);

		bson_error_t error;
		bson_t  *bson  = bson_new_from_json (json, -1, &error);
		if (!mongoc_collection_insert (record_match, MONGOC_INSERT_NONE, bson, NULL, &error)) {
			fprintf (stderr, "%s\n", error.message);
		}
	}

	/** 
	*	Destroy mongo connection.
	*/
	void destroy() {
		bson_destroy (_insert);
		bson_destroy (&reply);
		bson_destroy (command);
		bson_free (str);

		mongoc_collection_destroy (teams);
		mongoc_database_destroy (database);
		mongoc_client_destroy (client);
		mongoc_cleanup ();
	}

	/**	
	*	Checks if player 1 or 2 is in a match already.
	*	@param player1 string of first player's name.
	*	@param player2 string of second player's name.
	*	@return true if either player or 2 is in a match.  False otherwise.	
	*/
	bool is_in_match(char *player1, char *player2) {

		char *result = (char*) malloc( BUFSIZ * sizeof(char));

		//printf("TESTING IF IS IN MATCH\n");

		mongoc_collection_t *match_status = mongoc_client_get_collection (client, "go_server", "match_status");

		result = find(player1, player2, "status", match_status);
		if(strcmp(result, "ERROR") == 0) {
			printf("ERROR IN FIND DETECTED\n");
			result = find(player1, player2, "status", match_status);
		}

		if(strcmp(result, "GAME FINISHED") == 0) {
			return false;
		} else if(strcmp(result, "IN PROGRESS") == 0) {
			return true;
		} else if(strcmp(result, "NOT STARTED") == 0) {
			return false;
		}
		printf("RETURNING FALSE FAIL: %s\n", result);
		return  false;
	}
	
	/**
	*	Search for record in mongo databse and return one field according to key.  Must be in the form of: { "query1":"query2", ".........", "field":"RESULT", ".......... } @return RESULT.  query1, query2, field
	*	@param query1 and query2 defines the collection to be found in the form of query1:query2
	*	@param query2 
	*	@param field is the key of which data in the given record must be found.
	*	@param collection pointer to mongo database.
	*	@return pointer to char array of found data.  Or "FAIL" if not successful. 
	*/
	char* find(char *query1, char *query2, char *field, mongoc_collection_t *collection) {
		mongoc_cursor_t *cursor;
		const bson_t *doc; 	char *str; 	bool success = false;

		_query = bson_new();
		BSON_APPEND_UTF8 (_query, query1, query2);


		printf("COMMAND: %s:%s FIELD: %s\n", query1, query2, field);
		cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, _query, NULL, NULL);

		char *search = (char*) malloc( BUFSIZ * sizeof(char));
		strcpy(search, "\""); 		strcat(search, field); 		strcat(search, "\" : \"");

		char *result = (char*) malloc( BUFSIZ * sizeof(char));
		int j = 0; bool pass = false; string real;

		while (mongoc_cursor_next (cursor, &doc)) {
		str = bson_as_json (doc, NULL);
		for(int i = 0; i < strlen(str); i++) {
			if(search[j] == str[i] && !pass) {
				if(j+1 == strlen(search)) {
					pass = true;
					j = 0; continue;
				}
				j++;
			} else if(!pass) j = 0;

			if(pass) {
				if(str[i] == '"') { 
					result[j] = '\0';
					success = true;
					break;
				}
				result[j] = str[i];
				j++;
			}
		}
		bson_free (str);
		}

		if (mongoc_cursor_error (cursor, &error)) {
			count++; 
			fprintf (stderr, "An error occurred (find): %s: for command %s:%s finding %s reattempting the %d time\n", error.message, query1, query2, field, count);
			strcpy(result, "ERROR");
			return result;
		}

		   mongoc_cursor_destroy (cursor);
		   bson_destroy (_query);

		if(success) {
			return result;
		} else {
			strcpy(result, "FAIL");	
			return result;
		}
	}

	private:	
	string sha256(const string str)
	{
	    unsigned char hash[SHA256_DIGEST_LENGTH];
	    SHA256_CTX sha256;
	    SHA256_Init(&sha256);
	    SHA256_Update(&sha256, str.c_str(), str.size());
	    SHA256_Final(hash, &sha256);
	    stringstream ss;
	    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	    {
		ss << hex << setw(2) << setfill('0') << (int)hash[i];
	    }
	    return ss.str();
	}

	



};
