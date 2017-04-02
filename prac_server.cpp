#include <iostream>
#include <vector>
#include <stack>
#include <deque>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include "api.h"
#define no_players 2
using namespace std;

int captured;
void capturing(int game_board[19][19][2]);
void floodFill(int blackOrWhite, int game_board[19][19][2]);
void levelDown(int level, int to, int flood[19][19]);
void match(int player1, int player2);

int main(int argc, char **argv) {
	int welcomeSocket, socket_, bot_socket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddr.sin_family = AF_INET;

	serverAddr.sin_port = htons(7891);

	serverAddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("INADDR_ANY");

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

		if(listen(welcomeSocket,5)==0)
			printf("Listening for players\n");
		else
			printf("Error\n");

		addr_size = sizeof serverStorage;
		char buffer[BUFSIZ];

		for( ; ; ) {
			int pid;	
			socket_ = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
			printf("PLAYER has just joined\n");

			
			recv(socket_, buffer, 1024, 0);
			buffer[0] = 'P'; buffer[1] = 'S';
			send(socket_, buffer, 1024, 0);

			if((pid = fork()) > 0) bot_socket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);				
			else if( pid == 0) execl("/home/terminator/go_server/client","");

			printf("BOT has just joined\n");
			if((pid = fork()) > 0) {
				continue;
			} else if(pid == 0) {
					
				match(socket_, bot_socket);	

				//Comp Finished
				buffer[0] == 'C'; buffer[1] == 'F';
				send(socket_, buffer, 1024, 0);
				send(bot_socket, buffer, 1024, 0);
				close(socket_);close(bot_socket);
			}
		}



	return 0;
}

void match(int player1, int player2) {
	int turn = 1; //It is player1's turn first by default
	char buffer[1024]; 
	int game_board[19][19][2];
	for(int i = 0; i < 19; i++) 
		for(int j = 0; j < 19; j++) {
			game_board[i][j][0] = -1;
			game_board[i][j][1] = 0;
		}

	buffer[0] = 1;
	send(player1, buffer, 1024, 0);
	buffer[0] = 0;
	send(player2, buffer, 1024, 0);

	for(int i = 0; i < 1000; i++) {

		capturing(game_board);
		
		buffer[0] = 0;
		while(!buffer[0]) {
			//First players turn
			recv(player1, buffer, 1024, 0);
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
		buffer[0] = 0;
		while(!buffer[0]) {
			//second players turn
			recv(player2, buffer, 1024, 0);
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
	}
	//Game over
	buffer[0] == 'G'; buffer[1] == 'O';
	send(player1, buffer, 1024, 0);
	send(player2, buffer, 1024, 0);

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
