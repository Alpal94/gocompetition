#include "api.h"


int main(int argc, char** argv) {
	API serv;
	if(argc == 3) {
		serv.init(argv[1], argv[2]);
	} else {
		printf("Please enter your user name and password.  Exiting.\n");
		return 0;
	}

	srand(time(0));

	//A game starts 
	while(1) {
		if(serv.compFinished()) break;

		serv.game_init();
		
		printf("WORKING");
		printf("%d", serv.blackOrWhite());
		bool turn = !serv.blackOrWhite();

		int x; int y;
		if(turn) {
			x = rand() % 19;
			y = rand() % 19;
			
			serv.sendMove(x, y);
		
			turn = false;
		}
		else {
			serv.receiveMove();

			turn = true;
		}

		while(1) {
			serv.printGameBoard();
			if(turn) {
				printf("TURN\n");
				bool success = false;
				while(!success) {
					int x = rand() % 19;
					int y = rand() % 19;
				
					success = serv.sendMove(x, y);
					//printf("SUCCESS: %d", success);
					
					//Check if Game Over
					if(serv.gameOver()) break;
				}
				if(serv.gameOver()) break;
				turn = false;
			}
			else {
				printf("NOT TURN\n");
				char *buffer;
				buffer = serv.receiveMove();
				
				//Check if Game Over
				if(serv.gameOver()) break;
	
				turn = true;
			}
			if(serv.gameOver()) break;
		}
		printf("GAME IS OVER FOR CLIENT\n");
	}
}
