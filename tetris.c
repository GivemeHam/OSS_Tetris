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

#include "tetris.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// {{{ bricks
#define numBrickTypes 7
// positions of the filled cells
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// [brickNr][rotation][cellNr]
const unsigned char bricks[numBrickTypes][4][4] = {
	{ { 1,  5,  9, 13}, { 8,  9, 10, 11}, { 1,  5,  9, 13}, { 8,  9, 10, 11}, }, // I
	{ { 5,  6,  9, 10}, { 5,  6,  9, 10}, { 5,  6,  9, 10}, { 5,  6,  9, 10}, }, // O
	{ { 9,  8,  5, 10}, { 9,  5, 10, 13}, { 9, 10, 13,  8}, { 9, 13,  8,  5}, }, // T
	{ { 9, 10, 12, 13}, { 5,  9, 10, 14}, { 9, 10, 12, 13}, { 5,  9, 10, 14}, }, // S
	{ { 8,  9, 13, 14}, { 5,  8,  9, 12}, { 8,  9, 13, 14}, { 5,  8,  9, 12}, }, // Z
	{ { 5,  9, 12, 13}, { 4,  8,  9, 10}, { 5,  6,  9, 13}, { 8,  9, 10, 14}, }, // J
	{ { 5,  9, 13, 14}, { 8,  9, 10, 12}, { 4,  5,  9, 13}, { 6,  8,  9, 10}, }, // L
};
// }}}

static void dieIfOutOfMemory(void *pointer) { // {{{
	if (pointer == NULL) {
		printf("Error: Out of memory\n");
		exit(1);
	}
} // }}}

static void nextBrick(TetrisGame *game) { // {{{
	game->brick = game->nextBrick;
	game->brick.x = game->width/2 - 2;
	game->brick.y = 0;
	srand(time(NULL));
	game->nextBrick.type = rand() % numBrickTypes;
	game->nextBrick.rotation = rand() % 4;
	switch (game->nextBrick.type){
		case 0 : game->nextBrick.color = 1; 
			break;
		case 1 : game->nextBrick.color = 2;
			break;
		case 2 : game->nextBrick.color = 3;
			break;
		case 3 : game->nextBrick.color = 4;
			break;
		case 4 : game->nextBrick.color = 5;
			break;
		case 5 : game->nextBrick.color = 6;
			break;
		case 6 : game->nextBrick.color = 7;
			break;
		default : return ;
		}
	game->nextBrick.x = 0;
	game->nextBrick.y = 0;
} // }}}
int setLevel(){
	int level[5] = {500000, 400000, 300000, 200000, 100000};
	int select_level = 0;
	char c='\0';

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
void initGame(TetrisGame *game){
	dieIfOutOfMemory(game);
	game->width = 10;
	game->height = 20;
	game->size = game->width * game->height;
	game->board = calloc(game->size, sizeof(int));
	dieIfOutOfMemory(game->board);
	game->isRunning = 1;
	game->isPaused  = 0;
	game->sleepUsec = setLevel();
	game->score = 0;
	nextBrick(game); // fill preview
	nextBrick(game); // put into game
}

void initTerm(TetrisGame *game){
	struct termios term;
	tcgetattr(STDIN_FILENO, &game->termOrig);
	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void initSig(){
	struct sigaction signalAction;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_handler = signalHandler;
	signalAction.sa_flags = 0;
	sigaction(SIGINT,  &signalAction, NULL);
	sigaction(SIGTERM, &signalAction, NULL);
	sigaction(SIGSEGV, &signalAction, NULL);
	sigaction(SIGALRM, &signalAction, NULL);
}
void initTimer(TetrisGame *game){
	game->timer.it_value.tv_usec = game->sleepUsec;
	setitimer(ITIMER_REAL, &game->timer, NULL);
}
void destroyTetrisGame(TetrisGame *game) { // {{{
	if (game == NULL) return;
	tcsetattr(STDIN_FILENO, TCSANOW, &game->termOrig);
	printf("Your score: %li\n", game->score);
	printf("Game over.\n");
	free(game->board);
	free(game);
} // }}}

unsigned int xyToBrickXY(unsigned int brickXY,unsigned int xy){
	unsigned int rst = xy - brickXY;
	return rst;
}

unsigned int isOutBrick(unsigned int location)
{
	if(location < 0 )
		return 1;
	else if (location >= 4 )
		return 1;
	else
		return 0;
}

unsigned int xyToBricklocation(unsigned int x,unsigned int y){
	unsigned int rst = x + 4*y ;
	return rst;
}

unsigned int isBrickParticle(FallingBrick *brick,unsigned int location,unsigned int i){
	if (location == bricks[brick->type][brick->rotation][i])
		return 1;
	else
		return 0;
}

unsigned int colorOfBrickAt(FallingBrick *brick,unsigned int x,unsigned int y) { // {{{
	unsigned int brickXY = 0;
	unsigned int brickY;
	unsigned int brickX;

	if (brick->type < 0)
		return 0;

	brickY = xyToBrickXY(brick->y, y);
	if (isOutBrick(brickY))
		return 0;

	brickX = xyToBrickXY(brick->x, x);
	if (isOutBrick(brickX))
		return 0;

	brickXY = xyToBricklocation(brickX, brickY);
	for (unsigned int i = 0; i < 4; i++) {
		if (isBrickParticle(brick, brickXY, i))
			return brick->color;
	}
	return 0;
} // }}}


unsigned int particleToX(unsigned int p,unsigned int x){
	unsigned int particle = p % 4 + x;
	return particle;
}

unsigned int particleToY(unsigned int p,unsigned int y){
	unsigned int particle = p / 4 + y;
	return particle;
}

unsigned int xyTogameboard(unsigned int x,unsigned int y,unsigned int width){
	unsigned int rst = x + y * width;
	return rst;
}

unsigned int isOverlap(unsigned int particle, TetrisGame *game){
	if(particle < 0)
		return 0;
	if(particle >= game->size)
		return 0;
	if(game->board[particle] == 0)
		return 0;
	return 1;
}


unsigned int brickCollides(TetrisGame *game) { // {{{
	for (int i = 0; i < 4; i++) {
		unsigned int particle = bricks[game->brick.type][game->brick.rotation][i];
		unsigned int y = 0;
		unsigned int x = particleToX(particle , game->brick.x);
		if (x < 0 || x >= game->width)
			return 1;

		y = particleToY(particle, game->brick.y);
		if (y >= game->height)
			return 1;

		particle = xyTogameboard(x,y,game->width);
		if (isOverlap(particle,game))
			return 1;
	}
	return 0;
} // }}}

static void landBrick(TetrisGame *game) { // {{{
	unsigned int cell;
	unsigned int index;
	unsigned int temp;
	unsigned int x,y;
	
	if (game->brick.type < 0) return;
	
	for (cell = 0; cell < 4; cell++) {
		temp = bricks[game->brick.type][game->brick.rotation][cell];
		x = temp % 4 + game->brick.x;
		y = temp / 4 + game->brick.y;
		index = x + y * game->width;
		game->board[index] = game->brick.color;
	}
} 


static void clearFullRows(TetrisGame *game) { 
	unsigned int width = game->width;
	unsigned int rowsCleared = 0;
	unsigned int clearRow;
	unsigned int x;
	unsigned int y;
	unsigned int row;
	
	for (y = game->brick.y; y < game->brick.y + 4; y++) {
		clearRow = 1;
		for (x = 0; x < width; x++) {
			if (0 == game->board[x + y * width]) {
				clearRow = 0;
				break;
			}
		}
		if (clearRow) {
			for (row = y; row > 0; row--)
				memcpy(game->board + width*row, game->board + width*(row-1), width*sizeof(unsigned int));
			
			memset(game->board,0,width*sizeof(unsigned int));//instead bzero
			rowsCleared++;
		}
	}
	if (rowsCleared > 0) {
		// apply score: 0, 1, 3, 5, 8
		game->score += rowsCleared * 2 - 1;
		if (rowsCleared == 4) game->score++;
	}
} 

void tick(TetrisGame *game) { 
	if (game->isPaused) return;

	game->brick.y++;
	if (brickCollides(game)) {
		game->brick.y--;
		landBrick(game);
		clearFullRows(game);
		nextBrick(game);
		if (brickCollides(game))
			game->isRunning = 0;
	}

	printBoard(game);
} 

static void pauseUnpause(TetrisGame *game) { 
	if (game->isPaused) {
		// TODO de-/reactivate timer
		tick(game);
	}
	game->isPaused ^= 1;
}

unsigned int moveBrick(TetrisGame *game, unsigned int x, unsigned int y) { 
	if (game->isPaused)
		return 0;
	game->brick.x += x;
	game->brick.y += y;
	if (brickCollides(game)) {
		game->brick.x -=x;
		game->brick.y -=y;
		return 0;
	}
	printBoard(game);
	return 1;
} 

void changeRotation(TetrisGame *game,unsigned int direction){
	unsigned int rotate = game->brick.rotation + 4;

	if (direction == 1)
		rotate++;
	else
		rotate--;

	game->brick.rotation = rotate%4;
}
void rotateBrick(TetrisGame *game, unsigned int direction) { // {{{
	unsigned int oldRotation = game->brick.rotation;
	if (game->isPaused)
		return ;
	changeRotation(game,direction);
	if (brickCollides(game))
	{
		game->brick.rotation = oldRotation;
		return ;
	}
	printBoard(game);
} // }}}

void dropBrick(TetrisGame *game){
	while(moveBrick(game, 0 ,1));
}

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

