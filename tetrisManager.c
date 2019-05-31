#include "tetris.h"

void welcome() { // {{{
	printf("tetris-term  Copyright (C) 2014  Gjum\n");
	printf("OSS_Tetris team contributes this project.\n");
	printf("\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY.\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions; see `LICENSE' for details.\n");
	printf("\n");
	// Tetris logo
	printf("\e[30;40m  \e[31;41m  \e[30;40m  \e[34;44m  \e[34;44m  \e[34;44m  \e[33;43m  \e[30;40m  \e[30;40m  \e[30;40m  \e[37;47m  \e[35;45m  \e[35;45m  \e[35;45m  \e[39;49m\n");
	printf("\e[31;41m  \e[31;41m  \e[31;41m  \e[34;44m  \e[30;40m  \e[35;45m  \e[33;43m  \e[33;43m  \e[33;43m  \e[30;40m  \e[37;47m  \e[35;45m  \e[30;40m  \e[30;40m  \e[39;49m\n");
	printf("\e[30;40m  \e[36;46m  \e[30;40m  \e[35;45m  \e[35;45m  \e[35;45m  \e[32;42m  \e[30;40m  \e[31;41m  \e[31;41m  \e[37;47m  \e[34;44m  \e[34;44m  \e[34;44m  \e[39;49m\n");
	printf("\e[30;40m  \e[36;46m  \e[30;40m  \e[34;44m  \e[30;40m  \e[30;40m  \e[32;42m  \e[30;40m  \e[31;41m  \e[30;40m  \e[37;47m  \e[30;40m  \e[30;40m  \e[34;44m  \e[39;49m\n");
	printf("\e[30;40m  \e[36;46m  \e[36;46m  \e[34;44m  \e[34;44m  \e[34;44m  \e[32;42m  \e[32;42m  \e[31;41m  \e[30;40m  \e[35;45m  \e[35;45m  \e[35;45m  \e[35;45m  \e[39;49m\n");
	printf("\n");
	printf("\e[1mControls:\e[0m\n");
	printf("<Left>  move brick left\n");
	printf("<Right> move brick right\n");
	printf("<Up>    rotate brick clockwise\n");
	printf("<Down>  rotate brick counter-clockwise\n");
	printf("<d>     drop brick down\n");
	printf("<Space> move brick down by one step\n");
	printf("<p>     pause game\n");
	printf("<q>     quit game\n");
	printf("\n");
} // }}}

void playGame(){
	while(1){
		g_game =  newTetrisGame();
		// create space for the board
		for (int i = 0; i < g_game->height + 2; i++) printf("\n");
		printBoard(g_game);
		while (g_game->isRunning) {
			usleep(50000);
			processInputs(g_game);
		}
		g_game->sleepUsec = 0;
		sleep(3000);
		gameover(g_game);
		destroyTetrisGame(g_game);
	
		if(replay()) continue;
		else break;
	}
}

TetrisGame *newTetrisGame(unsigned int width, unsigned int height) { // {{{
	TetrisGame *game = malloc(sizeof(TetrisGame));
	initGame(game);
	// init terminal for non-blocking and no-echo getchar()
	initTerm(game);
	// init signals for timer and errors
	initSig();
	// init timer
	initTimer(game);
	return game;
} // }}}

void processInputs(TetrisGame *game) { // {{{
	int c = getchar();
	do {
		switch (c) {
			case ' ': moveBrick(game, 0, 1); break;
			case 'd': case 'D': dropBrick(game); break;
			case 'p': case 'P': pauseUnpause(game); break;
			case 'q': case 'Q': game->isRunning = 0; break;
			case 27: // ESC
				getchar();
				switch (getchar()) {
					case 'A': rotateBrick(game,  1);  break; // up
					case 'B': rotateBrick(game, 2);  break; // down
					case 'C': moveBrick(game, 1, 0); break; // right
					case 'D': moveBrick(game, -1, 0); break; // left
					default: break;
				}	
				break;
			default:  break;
		}
		c = getchar();
	} while (c != -1);
} // }}}

void destroyTetrisGame(TetrisGame *game) { // {{{
	if (game == NULL) return;
	tcsetattr(STDIN_FILENO, TCSANOW, &game->termOrig);
	printf("Your score: %li\n", game->score);
	printf("Game over.\n");
	free(game->board);
	free(game);
} // }}}

void pauseUnpause(TetrisGame *game) { 
	if (game->isPaused) {
		// TODO de-/reactivate timer
		tick(game);
	}
	game->isPaused ^= 1;
}

void gameover(TetrisGame *game){
	for(int i=0; i<game->size; i++)
		if(game->board[i]!=0) game->board[i] = game->nextBrick.color;
	printBoard(game);
}

int replay(){
	char replay = 'y';
	int c='\0';
	while(1){
		printf("replay? (y/n) :");
		scanf("%c", &replay);
		c=getchar();
		while(c != '\n') c=getchar();
		if(replay == 'y' || replay == 'n'
			|| replay == 'Y' || replay == 'N') break;
		printf("Insert Only 'y' or 'n'\n");
	}	
	if(replay == 'y' || replay =='Y') return 1;
	else return 0;
}


int setLevel(){
	int level[5] = {500000, 400000, 300000, 200000, 100000};
	int select_level = 0;
	int c='\0';

	while(1){	
		printf("Set Level(1~5): ");
		scanf("%d",&select_level);
		c=getchar();
		while(c != '\n') c=getchar();
		if(select_level<1 || select_level>5) {
			printf("[!!!]Insert 1-5\n");
		}
		else break;
	}
	return level[select_level-1];
}
