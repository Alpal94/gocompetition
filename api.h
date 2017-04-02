#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/**
*	API for client to server communication 
*/

class API {
	int clientSocket;
	char * buffer;
	struct sockaddr_in serverAddr;

	int **game_board;

	int _blackOrWhite;
	bool _gameOver; bool _compFinished; bool _accident;

	public:
	/** 	Initialises connection to game server and handles authentication.  
	*
	*	
	*	Exits on failure with error message.  Requires username and password for authentication.
	*
	*	@param username is requried to identify teams.  The username is created through the Go registration page at: gocompetition.hopto.org/go_server
	*
	*	@param password is required to help prevent accounts being hijacked by other teams on the day.  Entered in plain text.  Accounts can be registered at the Go registration page.
	*/
	void init (char *username, char* password) {
		socklen_t addr_size;

		clientSocket = socket(PF_INET, SOCK_STREAM, 0);

		serverAddr.sin_family = AF_INET;

		serverAddr.sin_port = htons(7891);

		serverAddr.sin_addr.s_addr = inet_addr("144.136.75.24");

		memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

		addr_size = sizeof serverAddr;
		if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) < 0) {
			printf("ERROR CONNECTION");;
		}
		buffer = (char*) malloc (1024 * sizeof(char));
		strcpy(buffer, username);strcat(buffer, ":");strcat(buffer,password);
		printf("BEING SENT: %s\n", buffer);
		send(clientSocket, buffer, 1024, 0);
		recv(clientSocket, buffer, 1024, 0);
		if(buffer[0] == 'F' && buffer[1] == 'F') { printf("\nInvalid password and/or username.  If you have forgotten your password, you can reregister at theterminator.hopto.org/go_server with a different username.  Otherwise, please try and reconnect with the correct username and password.\n\n"); exit(EXIT_FAILURE); }
		if(buffer[0] == 'S' && buffer[1] == 'S') { printf("\nConnection and authentication successfull.  Waiting on server to start the competition.\n"); }  
		if(buffer[0] == 'P' && buffer[1] == 'S') { printf("\nConnection to practice server.  Auth not required. Practice match about to commence.\n"); }  

		game_board = (int**) malloc(19 * sizeof(int*));
		for(int i = 0; i < 19; i++) game_board[i] = (int*) malloc(19*sizeof(int));

		for(int i = 0; i < 19; i++)
			for(int j = 0; j < 19; j++)
				game_board[i][j] = -1;
	}

	/**
	*	Initialises each game.  Communicates with server to ensure both server and client are ready to start the game.  Waits for server signal if server is not ready.  
	*
	*	If signal for competition completion is received terminates program. 
	*/
	void game_init() {
		while(1) {
			recv(clientSocket, buffer, 1024, 0);
			if(strcmp(buffer, "new game about to start") == 0) {
				printf("NEW GAME ABOUT TO START\n");
				strcpy(buffer, "client is ready to start game");
				send(clientSocket, buffer, 1024, 0); 
				break;
			}
			if(strcmp(buffer, "competition is over") == 0) {
				printf("COMPETITION IS OVER\n");
				exit(EXIT_SUCCESS);
				break;
			}
			printf("WARNING:  CLIENT IS READY FOR NEW GAME.  SERVER IS NOT.  WAITING FOR SERVER SIGNAL\n");
			sleep(1);
		}

		recv(clientSocket, buffer, 1024, 0);

		//Black is represented by 0
		if(buffer[0]) _blackOrWhite = 0;
		else _blackOrWhite = 1;
	}

	/**
	*	 @return bots playing colour.  Returns 0 for black, 1 for white.  Black moves first.
	*/
	bool blackOrWhite() { return _blackOrWhite; }
	
	/**
	*	@eturn true if competition is finished.
	*/
	bool compFinished() { return _compFinished; }

	/**
	*	@return true if game is over.
	*/
	bool gameOver() { return _gameOver; }

	/**
	*	@return pointer to game board.  Game board is a char array.  * for black, o for white, . for no stones.
	*/
	int **returnGameBoard() { return game_board; }

	/**
	*	Sends move to game server.  Two intergers representing the x,y position on the game board. 
	*	@return Returns true if move is accepted by server, false if move is invalid.
	*/
	bool sendMove(int x, int y) {
		buffer[0] = x; buffer[1] = y;
		//Send turn info 
		send(clientSocket, buffer, 1024, 0);
		//Server will check and reply if turn was successfull
		recv(clientSocket, buffer, 1024, 0);

		//Check if Game Over
		gameOverPriv(buffer);
		if(gameOver()) {
			printf("\n\nCLIENT HAS RECIEVED GAME OVER\n\n");
			return true;
		}

		//Server returns 1 on success.  Return bool if move is legal and accepted, false otherwise
		bool success = buffer[0];
		if(success) game_board[x][y] = _blackOrWhite;
		return success;
	}

	/**
	*	Receives move made by other player from the server.  Returns char array of integers.  First index is the x coord second index is y coord.
	*
	*	char *move = receiveMove(); <br>
	*	int x = move[0];  <br>
	*	int y = move[1];  <br>
	*
	*	@return char array of integers 
	*/
	char *receiveMove() {
		recv(clientSocket, buffer, 1024, 0);

		//Check if Game Over
		gameOverPriv(buffer);
		if(gameOver()) {
			printf("\n\nCLIENT HAS RECIEVED GAME OVER\n\n");
			return buffer;
		}

		game_board[buffer[0]][buffer[1]] = (_blackOrWhite + 1)%2;
		return buffer;
	}

	/**
	*	Prints game board.  Useful for debugging.  
	*/
	void printGameBoard() {
		for(int i = 0; i < 19; i++) {
			for(int j = 0; j < 19; j++) {
				if(game_board[i][j] == 0) printf(" *");
				if(game_board[i][j] == 1) printf(" o");
				if(game_board[i][j] == -1)printf(" .");
			}
			printf("\n");
		}
	}

	private:
	bool gameOverPriv(char *buffer) {
		if(buffer[0] == 'G' && buffer[1] == 'O') { _gameOver = true; return true; }
		if(buffer[0] == 'n' && buffer[1] == 'e') { _gameOver = true; _accident = true; return true; }
		_gameOver = false; return false;
	}

	bool compFinishedPriv(char *buffer) {
		if(buffer[0] == 'C' && buffer[1] == 'F') { _compFinished = true; return true; }
		_compFinished = false; return false;
	}
};
