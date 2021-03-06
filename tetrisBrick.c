#include "tetris.h"

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

void printBoard(TetrisGame *game) { // {{{
	unsigned int color = 0;
	char line[game->width * 2 + 1];
	memset(line, '-', game->width * 2);
	line[game->width * 2] = 0;
	printf("\e[%iA", game->height + 2); // move to above the board
	printf("/%s+--------\\\n", line);
	for (unsigned int y = 0; y < game->height; y++) {
		printf("|");
		for (unsigned int x = 0; x < game->width; x++) {
			color = game->board[x + y * game->width];
			if (color == 0) // empty? try falling brick
				color = colorOfBrickAt(&game->brick, x, y);
			printf("\e[3%i;4%im  ", color, color);
		}
		if (y <= 6 )
		{
			switch(y){
				case 4: printf("\e[39;49m|  \e[1mScore\e[0m |\n"); break;
				case 5: printf("\e[39;49m| %6li |\n", game->score); break;
				case 6: printf("\e[39;49m+--------/\n"); break;
				default: 
					printf("\e[39;49m|");
					for (unsigned int x = 0; x < 4; x++) {
						color = colorOfBrickAt(&game->nextBrick, x, y);
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


void nextBrick(TetrisGame *game) { // {{{
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

void landBrick(TetrisGame *game) { // {{{
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


void clearFullRows(TetrisGame *game) { 
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
		if (brickCollides(game)){
			game->isRunning = 0;
		}
	}

	printBoard(game);
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