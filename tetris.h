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

#include <termios.h>
#include <sys/time.h>
#include <time.h>

typedef struct { // FallingBrick {{{
	unsigned int type, rotation, color;
	unsigned int x, y;
} FallingBrick; // }}}

typedef struct { // TetrisGame {{{
	unsigned int width, height, size; // of the board
	unsigned char *board; // indices of pattern
	FallingBrick brick, nextBrick;
	unsigned char isRunning, isPaused;
	suseconds_t sleepUsec;
	unsigned long score;
	struct termios termOrig;
	struct itimerval timer;
} TetrisGame; // }}}

extern void signalHandler(int signal);
extern void printBoard(TetrisGame *game);
extern void playGame();
extern int replay();

TetrisGame *newTetrisGame();
void *initGame(TetrisGame *game);
void initTerm(TetrisGame *game);
void initSig();
void initTimer(TetrisGame *game);

void destroyTetrisGame(TetrisGame *game);
void processInputs(TetrisGame *game);
void tick(TetrisGame *game);
unsigned char colorOfBrickAt(FallingBrick *brick,unsigned int x,unsigned int y);

//Added functions
unsigned int xyToBrickXY(unsigned int brickXY,unsigned int xy);
unsigned int isOutBrick(unsigned int location);
unsigned int xyToBricklocation(unsigned int x,unsigned int y);
unsigned int isBrickParticle(FallingBrick *brick,unsigned int location,unsigned int i);
unsigned int particleToX(unsigned int p,unsigned int x);
unsigned int particleToY(unsigned int p,unsigned int y);
unsigned int xyTogameboard(unsigned int x,unsigned int y,unsigned int width);
unsigned int isOverlap(unsigned int particle, TetrisGame *game);
void changeRotation(TetrisGame * game,char direction);
