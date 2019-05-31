/* tetris-term - Classic Tetris for your terminal.
 *
 * Copyright (C) 2014 Gjum
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "tetris.h"

TetrisGame *game;

void printBoard(TetrisGame *gamep) { // {{{
	unsigned int color = 0;
	char line[gamep->width * 2 + 1];
	memset(line, '-', gamep->width * 2);
	line[gamep->width * 2] = 0;
	printf("\e[%iA", gamep->height + 2); // move to above the board
	printf("/%s+--------\\\n", line);
	for (unsigned int y = 0; y < gamep->height; y++) {
		printf("|");
		for (unsigned int x = 0; x < gamep->width; x++) {
			color = gamep->board[x + y * gamep->width];
			if (color == 0) // empty? try falling brick
				color = colorOfBrickAt(&gamep->brick, x, y);
			printf("\e[3%i;4%im  ", color, color);
		}
		if (y <= 6 )
		{
			switch(y){
				case 4: printf("\e[39;49m|  \e[1mScore\e[0m |\n"); break;
				case 5: printf("\e[39;49m| %6li |\n", gamep->score); break;
				case 6: printf("\e[39;49m+--------/\n"); break;
				default: 
					printf("\e[39;49m|");
					for (unsigned int x = 0; x < 4; x++) {
						color = colorOfBrickAt(&gamep->nextBrick, x, y);
						printf("\e[3%i;4%im  ", color, color);
					}
					printf("\e[39;49m|\n");
			}
		}
		else
			printf("\e[39;49m|\n");
	}
	printf("\\%s/\n", line);
} // }}}

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

void signalHandler(int signal) { // {{{
	switch(signal) {
		case SIGINT:
		case SIGTERM:
		case SIGSEGV:
			game->isRunning = 0;
			break;
		case SIGALRM:
			tick(game);
			game->timer.it_value.tv_usec = game->sleepUsec;
			setitimer(ITIMER_REAL, &game->timer, NULL);
			break;
	}
	return;
} // }}}

int main(int  argc, char **argv) { // {{{
	srand(time(0));
	welcome();
	playGame();
	return 0;
} // }}}
void playGame(){
	while(1){
		game =  newTetrisGame();
		// create space for the board
		for (int i = 0; i < game->height + 2; i++) printf("\n");
		printBoard(game);
		while (game->isRunning) {
			usleep(50000);
			processInputs(game);
		}
		game->sleepUsec = 0;
		sleep(3000);
		gameover(game);
		destroyTetrisGame(game);
	
		if(replay()) continue;
		else break;
	}
}

void gameover(TetrisGame *games){
	for(int i=0; i<games->size; i++)
		if(games->board[i]!=0) games->board[i] = games->nextBrick.color;
	printBoard(games);
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
