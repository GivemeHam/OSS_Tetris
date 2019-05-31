#include "tetris.h"

int main(int  argc, char **argv) { // {{{
	srand(time(0));
	welcome();
	playGame();
	return 0;
} // }}}