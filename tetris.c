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
		}game->nextBrick.x = 0;
	game->nextBrick.y = 0;
} // }}}

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
void *initGame(TetrisGame *game){
	dieIfOutOfMemory(game);
	game->width = 10;
	game->height = 20;
	game->size = game->width * game->height;
	game->board = calloc(game->size, sizeof(char));
	dieIfOutOfMemory(game->board);
	game->isRunning = 1;
	game->isPaused  = 0;
	game->sleepUsec = 500000;
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
	printf("Your score: %i\n", game->score);
	printf("Game over.\n");
	free(game->board);
	free(game);
} // }}}

int xyToBrickXY(int brickXY, int xy){
	int rst = xy - brickXY;
	return rst;
}

int isOutBrick(int location)
{
	if(location < 0 )
		return 1;
	else if (location >= 4 )
		return 1;
	else
		return 0;
}

int xyToBricklocation(int x,int y){
	int rst = x + 4*y ;
	return rst;
}

int isBrickParticle(FallingBrick *brick, int location,int i){
	if (location == bricks[brick->type][brick->rotation][i])
		return 1;
	else
		return 0;
}

unsigned char colorOfBrickAt(FallingBrick *brick, int x, int y) { // {{{
	int brickXY = 0;
	
	if (brick->type < 0)
		return 0;

	int brickY = xyToBrickXY(brick->y, y);
	if (isOutBrick(brickY))
		return 0;

	int brickX = xyToBrickXY(brick->x, x);
	if (isOutBrick(brickX))
		return 0;

	brickXY = xyToBricklocation(brickX, brickY);
	for (int i = 0; i < 4; i++) {
		if (isBrickParticle(brick, brickXY, i))
			return brick->color;
	}
	return 0;
} // }}}


int particleToX(int p, int x){
	int particle = p % 4 + x;
	return particle;
}

int particleToY(int p, int y){
	int particle = p / 4 + y;
	return particle;
}

int xyTogameboard(int x, int y, int width){
	int rst = x + y * width;
	return rst;
}

int isOverlap(int particle, TetrisGame *game){
	if(particle < 0)
		return 0;
	if(particle >= game->size)
		return 0;
	if(game->board[particle] == 0)
		return 0;
	return 1;
}


static char brickCollides(TetrisGame *game) { // {{{
	for (int i = 0; i < 4; i++) {
		int particle = bricks[game->brick.type][game->brick.rotation][i];
		int x = particleToX(particle , game->brick.x);
		if (x < 0 || x >= game->width)
			return 1;

		int y = particleToY(particle, game->brick.y);
		if (y >= game->height)
			return 1;

		particle = xyTogameboard(x,y,game->width);
		if (isOverlap(particle,game))
			return 1;
	}
	return 0;
} // }}}

static void landBrick(TetrisGame *game) { // {{{
	int cell;
	int index;
	int temp;
	int x,y;
	
	if (game->brick.type < 0) return;
	
	for (cell = 0; cell < 4; cell++) {
		temp = bricks[game->brick.type][game->brick.rotation][cell];
		x = temp % 4 + game->brick.x;
		y = temp / 4 + game->brick.y;
		index = x + y * game->width;
		game->board[index] = game->brick.color;
	}
} // }}}


static void clearFullRows(TetrisGame *game) { // {{{
	int width = game->width;
	int rowsCleared = 0;
	int clearRow;
	int x;
	int y;
	int row;
	
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
				memcpy(game->board + width*row, game->board + width*(row-1), width);
			
			memset(game->board,0,width);//instead bzero
			rowsCleared++;
		}
	}
	if (rowsCleared > 0) {
		// apply score: 0, 1, 3, 5, 8
		game->score += rowsCleared * 2 - 1;
		if (rowsCleared == 4) game->score++;
	}
} // }}}

void tick(TetrisGame *game) { // {{{
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
} // }}}

static void pauseUnpause(TetrisGame *game) { // {{{
	if (game->isPaused) {
		// TODO de-/reactivate timer
		tick(game);
	}
	game->isPaused ^= 1;
} // }}}

int moveBrick(TetrisGame *game, char x, char y) { // {{{
	if (game->isPaused) return;
	game->brick.x += x;
	game->brick.y += y;
	if (brickCollides(game)) {
		game->brick.x -= x;
		game->brick.y -= y;
		return 0;
	}
	printBoard(game);
	return 1;
} // }}}

static void changeRotation(TetrisGame *game,char direction){
	game->brick.rotation = (game->brick.rotation + 4 + direction)%4;
}

static void rotateBrick(TetrisGame *game, char direction) { // {{{
	if (game->isPaused) return;
	unsigned char oldRotation = game->brick.rotation;
	changeRotation(game,direction);
	if (brickCollides(game))
	{
		game->brick.rotation = oldRotation;
		return ;
	}
	printBoard(game);
} // }}}

static void dropBrick(TetrisGame *game){
	while(moveBrick(game, 0 ,1));
}

void processInputs(TetrisGame *game) { // {{{
	char c = getchar();
	do {
		switch (c) {
			case ' ': moveBrick(game, 0, 1); break;
			case 'd': dropBrick(game); break;
			case 'p': pauseUnpause(game); break;
			case 'q': game->isRunning = 0; break;
			case 27: // ESC
				getchar();
				switch (getchar()) {
					case 'A': rotateBrick(game,  1);  break; // up
					case 'B': rotateBrick(game, -1);  break; // down
					case 'C': moveBrick(game,  1, 0); break; // right
					case 'D': moveBrick(game, -1, 0); break; // left
				}
				break;
		}
	} while ((c = getchar()) != -1);
} // }}}

