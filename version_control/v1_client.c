#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main() {
	int clientSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;

	srand(time(0));

	int game_board[19][19];
	for(int i = 0; i < 19; i++)
		for(int j = 0; j < 19; j++)
			game_board[i][j] = -1;

	socklen_t addr_size;

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddr.sin_family = AF_INET;

	serverAddr.sin_port = htons(7891);

	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	addr_size = sizeof serverAddr;
	connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
	//Check to see set up info from server --> 1 your go to start (black) 0 --> opponent goes first
	recv(clientSocket, buffer, 1024, 0);
	printf("WORKING");
	printf("%d", buffer[0]);

	bool turn = false;	
	bool blackOrWhite = 0;
	int x; int y;
	if(buffer[0] == 1) {
		turn = true;
		x = rand() % 19;
		y = rand() % 19;
		
		buffer[0] = x; buffer[1] = y;
		game_board[x][y] = 1;
		//Send turn info 
		send(clientSocket, buffer, 1024, 0);
		//Server will check and reply if turn was successfull
		recv(clientSocket, buffer, 1024, 0);

		//Black is represented as zero
		blackOrWhite = 0;
		turn = false;
	}
	else {
		turn = false;
		recv(clientSocket, buffer, 1024, 0);
		game_board[buffer[0]][buffer[1]] = 1;

		//White is represented as 1
		blackOrWhite = 1;
		turn = true;
	}

	while(1) {
		if(turn) {
			buffer[0] = false;
			while(!buffer[0]) {
				x = rand() % 19;
				y = rand() % 19;
			
				buffer[0] = x, buffer[1] = y;
				
				send(clientSocket, buffer, 1024, 0);
				recv(clientSocket, buffer, 1024, 0);
			}
			game_board[x][y] = blackOrWhite;
			turn = false;
		}
		else if(!turn) {
			recv(clientSocket, buffer, 1024, 0);
			game_board[buffer[0]][buffer[1]] = (blackOrWhite + 1)%2;
			turn = true;
		}
		else break;
	}

}
