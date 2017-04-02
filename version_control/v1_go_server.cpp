#include <iostream>
#include <vector>
#include <stack>
#include <deque>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
using namespace std;

int captured;
void capturing(int game_board[19][19][2]);
void floodFill(int blackOrWhite, int game_board[19][19][2]);
void levelDown(int level, int to, int flood[19][19]);

int main(int argc, char **argv) {
	int welcomeSocket, socket1, socket2;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddr.sin_family = AF_INET;

	serverAddr.sin_port = htons(7891);

	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	while(1) {
		if(listen(welcomeSocket,5)==0)
			printf("Listening\n");
		else
			printf("Error\n");

		addr_size = sizeof serverStorage;
		socket1 = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
		socket2 = accept(welcomeSocket, (struct sockaddr *) & serverStorage, &addr_size);

		printf("new connection\n");
		
		int turn = 1; //It is socket1's turn first by default
		int game_board[19][19][2];
		for(int i = 0; i < 19; i++) 
			for(int j = 0; j < 19; j++) {
				game_board[i][j][0] = -1;
				game_board[i][j][1] = 0;
			}

		buffer[0] = 1;
		send(socket1, buffer, 1024, 0);
		buffer[0] = 0;
		send(socket2, buffer, 1024, 0);
	
		while(1) {
			printf("IS working\n");
			for(int i = 0; i < 19; i++) { 
				for(int j = 0; j < 19; j++)
					printf(" %d ", game_board[i][j][0]);
				printf("\n");
			}
			capturing(game_board);
			
			buffer[0] = 0;
			while(!buffer[0]) {
				//First players turn
				recv(socket1, buffer, 1024, 0);
				if(game_board[buffer[0]][buffer[1]][0] == -1) {
					game_board[buffer[0]][buffer[1]][0] = 1;
					send(socket2, buffer, 1024, 0);

					buffer[0] = 1;
					send(socket1, buffer, 1024, 0);
				}
				else {
					buffer[0] = 0;
					send(socket1, buffer, 1024, 0);
				}	
			}
			buffer[0] = 0;
			while(!buffer[0]) {
				//second players turn
				recv(socket2, buffer, 1024, 0);
				if(game_board[buffer[0]][buffer[1]][0] == -1) { 
					game_board[buffer[0]][buffer[1]][0] = 0;
					send(socket1, buffer, 1024, 0);
					
					buffer[0] = 1;
					send(socket2, buffer, 1024, 0);
				} else {
					buffer[0] = 0;
					send(socket2, buffer, 1024, 0);
				}
			}
		}
	}
	return 0;
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
