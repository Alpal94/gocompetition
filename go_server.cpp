#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <sys/types.h>
#include <sys/wait.h>
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
#include "mongo.h"
#include "match.h"
using namespace std;

/*int captured;
void capturing(int game_board[19][19][2]);
void floodFill(int blackOrWhite, int game_board[19][19][2]);
void levelDown(int level, int to, int flood[19][19]);
void match(int player1, int player2); 
void gameOver(int player1, int player2, char *buffer);*/

int main(int argc, char **argv) {
	int no_players = argv[1][0] - '0'; int welcomeSocket, socket_[no_players];
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;
	char buffer[BUFSIZ];
	char *live_teams[no_players];

	MONGO mongo;
	MATCH match;
	mongo.init();
	
	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(7891);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero); 
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	while(1) {
		if(listen(welcomeSocket,5)==0)
			printf("Listening for %d players\n", no_players);
		else
			printf("Error\n");

		addr_size = sizeof serverStorage;

		for(int i = 0; i < no_players;         ) {
			socket_[i] = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
			printf("PLAYER %d has just joined\n", i);
			recv(socket_[i], buffer, 1024, 0);
			for(int j = 0; j < BUFSIZ; j++) {
				if(buffer[j] == ':') {
					buffer[j] = '\0';
					if(mongo.auth_team( buffer , buffer+j+1 )) {
						printf("authentication is successfull for %s\n", buffer);
						live_teams[i] = (char*) malloc(strlen(buffer) * sizeof(char)); 
						strcpy(live_teams[i], buffer);
						buffer[0] = 'S'; buffer[1] = 'S';
						send(socket_[i], buffer, 1024, 0);
						mongo.record_team(live_teams[i]); i++;
					}
					else { 
						printf("invalid password for %s\n", buffer);
						buffer[0] = 'F'; buffer[1] = 'F';
						send(socket_[i], buffer, 1024, 0);
						close(socket_[i]);
					}
					break;
				}
			}
		}
		printf("\nTHE LIVE TEAMS ARE: ");
		for(int i = 0; i < no_players; i++) 
			printf("%s, ", live_teams[i]);

		bool is_in_match[no_players]; bool curr_opponent[no_players];
		for(int i = 0; i < no_players; i++)  { is_in_match[i] = false;  curr_opponent[i] = -1; }


		queue < pair < int, int > > matches_queue;
		for(int i = 0; i < no_players; i++) 
			for(int j = 0; j < no_players; j++) {
				if(i == j) continue;
				mongo.match_status_setup( live_teams[ i ] , live_teams[ j ] );
				printf("Match for %s vs %s in queue\n", live_teams[ i ] , live_teams[ j ] );
				pair < int, int > match( i , j );
				matches_queue.push( match );	
			}

		queue < pair < int, pair < int, int> > > pid_record;
		while(!matches_queue.empty()) {
			pair < int , int > curr = matches_queue.front();
			matches_queue.pop();
			printf("CHECKING IF MATCH IS VALID\n");
			if(curr_opponent[curr.first] != -1) if(
				mongo.is_in_match(	live_teams[curr.first], 		live_teams[curr_opponent[curr.first]]) 	||
				mongo.is_in_match(	live_teams[curr_opponent[curr.first]], 	live_teams[curr.first]) 			
				) {
				matches_queue.push(curr);
				continue;
			}
			if(curr_opponent[curr.second] != -1) if(
				mongo.is_in_match(	live_teams[curr.second], 		live_teams[curr_opponent[curr.second]])	||
				mongo.is_in_match(	live_teams[curr_opponent[curr.second]],	live_teams[curr.second]) 
				) {
				matches_queue.push(curr);
				continue;
			}
			sleep(1);
			printf("MATCH COULD START\n");
			is_in_match[curr.first] = true; 		is_in_match[curr.second] = true;
			curr_opponent[curr.first] = curr.second; 	curr_opponent[curr.second] = curr.first;
			mongo.match_status(live_teams[curr.first], live_teams[curr.second], 'P');

			int pid;
			if((pid = fork()) > 0) {
				pair < int, pair < int, int> > record;
				record.first = pid; record.second.first = curr.first; record.second.second = curr.second;

				queue < pair < int, pair < int, int> > > temp;
				while(!pid_record.empty()) {		
					pair < int, pair < int, int> > check = pid_record.front();
					pid_record.pop();
					if( !(check.second.first == curr.first || check.second.second == curr.first || check.second.first == curr.second || check.second.second == curr.second )) temp.push( check );
				}
				while(!temp.empty()) {
					pid_record.push( temp.front() );	
					temp.pop();
				}
				pid_record.push( record );
				continue;
			}
			else if(pid == 0) {
				printf("MATCH STARTING\n");
				match.match(socket_[curr.first], socket_[curr.second], live_teams[curr.first], live_teams[curr.second], mongo);
				mongo.match_status(live_teams[curr.first], live_teams[curr.second], 'F');

				cout << "MATCH FOR: " << live_teams[curr.first] << live_teams[curr.second] << " HAS JUST FINISHED\n";
				curr_opponent[curr.first] = -1; 	curr_opponent[curr.second] = -1;
				exit(1);	
			}
		}

		//Comp Finished
		while(!pid_record.empty()) {
			int status;
			pair < int, pair < int, int> > record = pid_record.front();
			pid_record.pop();
			
			printf("checking PID: %d for player: %d and %d\n", record.first, record.second.first, record.second.second );

			while(waitpid( record.first , &status, WNOHANG) == 0) { printf("WAITING\n"); sleep(1); }

			strcpy(buffer, "competition is over");
			send(socket_[record.second.first], buffer, 1024, 0);
			send(socket_[record.second.second], buffer, 1024, 0);
			printf("Players %d and %d have been notified that the competition is over\n", record.second.first, record.second.second );
		}
		cout << endl << "COMPETITION HAS FINISHED" << endl;

		printf("clients have been notified that competition has ended\n");

		return 0;
	}
}
