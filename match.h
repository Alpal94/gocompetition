#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

class MATCH {
	int captured;

	public:
	/**
 	*	Handles a match between two players. 
	*/
	void match(int player1, int player2, char* name_player_1, char* name_player_2, MONGO mongo) {
		int turn = 1; //It is player1's turn first by default
		char buffer[1024]; 
		int game_board[19][19][2];


		//Setup gameboard
		for(int i = 0; i < 19; i++) 
			for(int j = 0; j < 19; j++) {
				game_board[i][j][0] = -1;
				game_board[i][j][1] = 0;
			}

		//Yell each player game is about to start
		strcpy(buffer, "new game about to start");
		send(player1, buffer, 1024, 0);
		send(player2, buffer, 1024, 0);
		
		//Check player is ready for match
		recv(player1, buffer, 1024, 0);
		if(!strcmp(buffer, "client is ready to start game") == 0) {
			printf("CLIENT %d IS NOT READY TO START MATCH (first player) \n", player1);
			return;
		}

		recv(player2, buffer, 1024, 0);
		if(!strcmp(buffer, "client is ready to start game") == 0) {
			printf("CLIENT %d IS NOT READY TO START MATCH (second player) \n", player2);
			return;
		}


		//Tell palyers whether they are black or white.  Black starts first, black is represented by number 0.  White is 1.
		buffer[0] = 1;
		send(player1, buffer, 1024, 0);
		buffer[0] = 0;
		send(player2, buffer, 1024, 0);

		bool game_over = false;
		bool game_completely_over = false;

		for(int i = 0;    ; i++) {
			/*for(int i = 0; i < 19; i++) { 
				for(int j = 0; j < 19; j++)
					printf(" %d ", game_board[i][j][0]);
				printf("\n");
			}*/
			/*printf("\n");
			for(int i = 0; i < 19; i++) {
				for(int j = 0; j < 19; j++) {
					if(game_board[i][j][0] == 0) printf(" *");
					if(game_board[i][j][0] == 1) printf(" o");
					if(game_board[i][j][0] == -1)printf(" .");
				}
				printf("\n");
			}
			printf("\n");*/

			char board_string[BUFSIZ];
			char tmp[4];
			
			//Send board to mongo database
			strcpy(board_string, "{");
			for(int a = 0; a < 19; a++) {
				if(a < 10) { tmp[0] = a + '0';	tmp[1] = '\0'; }
				else { tmp[0] = 1 + '0';	tmp[1] = a - 10 + '0';	 tmp[3] = '\0'; }

				strcat(board_string, "\""); strcat(board_string, tmp); strcat(board_string, "\":{");
				for(int j = 0; j < 19; j++) {
					if(j < 10) { tmp[0] = j + '0';	tmp[1] = '\0'; }
					else { tmp[0] = 1 + '0';	tmp[1] = j - 10 + '0';	 tmp[3] = '\0'; }

					strcat(board_string, "\""); strcat(board_string, tmp); strcat(board_string, "\":\"");
					if(game_board[i][j][0] == 0) strcat(board_string, "*\"");
					if(game_board[i][j][0] == 1) strcat(board_string, "o\"");
					if(game_board[i][j][0] == -1)strcat(board_string, ".\"");
		
					if(j != 18) strcat(board_string, ",");
				}
				strcat(board_string, "}");
				if(a != 18) strcat(board_string, ",");
			}
			strcat(board_string, "}");

			tmp[0] = '0' + i;
			
			printf(" THE PLAYERS ARE JSON: %s and %s", name_player_1, name_player_2);
			sprintf(buffer, "move_%d", i);
			mongo.record_match(name_player_1, name_player_2, buffer, reinterpret_cast<unsigned char*>(board_string));


			//For testing purposes game is limited to 100 turns.
			if(i > 100) game_over = true;

			capturing(game_board);
			
			buffer[0] = 0;
			while(!buffer[0]) {
				//First players turn
				recv(player1, buffer, 1024, 0);

				if(game_over) {
					gameOver(player1, player2, buffer);
					game_completely_over = true;
					break;
				}

				if(game_board[buffer[0]][buffer[1]][0] == -1) {
					game_board[buffer[0]][buffer[1]][0] = 1;

					send(player2, buffer, 1024, 0);

					buffer[0] = 1;
					send(player1, buffer, 1024, 0);
				}
				else {
					buffer[0] = 0;
					send(player1, buffer, 1024, 0);
				}	
			}
		
			//Capture any dead stones.
			capturing(game_board);

			if(game_completely_over) break;
			buffer[0] = 0;
			while(!buffer[0]) {
				//second players turn
				recv(player2, buffer, 1024, 0);

				if(game_over) {
					gameOver(player1, player2, buffer);
					game_completely_over = true;
					break;
				}

				if(game_board[buffer[0]][buffer[1]][0] == -1) { 
					game_board[buffer[0]][buffer[1]][0] = 0;
					send(player1, buffer, 1024, 0);
					
					buffer[0] = 1;
					send(player2, buffer, 1024, 0);
				} else {
					buffer[0] = 0;
					send(player2, buffer, 1024, 0);
				}
			}

			//Capture any dead stones.
			capturing(game_board);

			if(game_completely_over) break;
		}
		printf("FOR PLAYER %d and %d the game has FINISHED\n", player1, player2 );
		
	}

	private:
	void gameOver(int player1, int player2, char *buffer) {
		//Game over
		buffer[0] = 'G'; buffer[1] = 'O';
		send(player1, buffer, 1024, 0);
		send(player2, buffer, 1024, 0);

		printf("GAME HAS ENDED!!!\n");
	}

	void floodFill(int blackOrWhite, int game_board[19][19][2]) {
		int flood[19][19];   int opponent = (blackOrWhite + 1)%2;
		int visited[19][19]; int level = 1; stack < pair < int, int > > thestack;
		for(int i = 0; i < 19; i++) for(int j = 0; j < 19; j++) { visited[i][j] = 0; flood[i][j] = 0; }

		for(int x = 0; x < 19; x++) 
			for(int y = 0; y < 19; y++) {

			if(visited[x][y]) continue;
			pair < int, int > coords ( x, y ); visited[x][y] = 1; thestack.push(coords);
			//cout << coords.first << "  " << coords.second << endl;
			
			while(!thestack.empty()) {
				pair <int, int> curr = thestack.top();
				thestack.pop(); visited[curr.first][curr.second] = 1;
				//cout << "\nTHE SIZE OF THE STACK IS: " << thestack.size() << " DEBUG IS: " << curr.first << " " << curr.second << endl;
				//cout << "\nBLACK OR WHITE IS: " << blackOrWhite << endl;
				if(game_board[curr.first][curr.second][0]  == -1) {
					visited[curr.first][curr.second] = 0;
					levelDown(level, 0, flood);
					while(!thestack.empty()) thestack.pop();
					break;
				}
				else if(game_board[curr.first][curr.second][0] == blackOrWhite) continue;
				else if(game_board[curr.first][curr.second][0] == opponent) {
					pair <int, int> next; flood[curr.first][curr.second] = level;
					if(curr.first + 1 < 19)   if(!visited[curr.first+1][curr.second]) {
						next.first = curr.first + 1; 	next.second = curr.second;
						thestack.push(next);
					} if(curr.second + 1 < 19)if(!visited[curr.first][curr.second + 1]) {
						next.first = curr.first; 	next.second = curr.second + 1;
						thestack.push(next);
					} if(curr.first - 1 >= 0) if(!visited[curr.first-1][curr.second]) {
						next.first = curr.first - 1; 	next.second = curr.second;
						thestack.push(next);
					} if(curr.second - 1 >= 0)if(!visited[curr.first][curr.second-1]) {
						next.first = curr.first; 	next.second = curr.second - 1;
						thestack.push(next);
					}
				}
			}
			level++;
		}
		
		//Remove all entrapped stones from the game board
		for(int i = 0; i < 19; i++)
			for(int j = 0; j < 19; j++) 
				if(flood[i][j] > 0) game_board[i][j][0] = -1;
	}

	//Either make one level the same as another or delete a level
	void levelDown(int level, int to, int flood[19][19]) {
		for(int i = 0; i < 19; i++) 
			for(int j = 0; j < 19; j++) 
				if(flood[i][j] == level) flood[i][j] = to;
	}

	void capturing(int game_board[19][19][2]) {
		floodFill(1, game_board);
		floodFill(0, game_board);
	}
};
