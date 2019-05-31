#include "tetris.h"

static void dieIfOutOfMemory(void *pointer) { // {{{
	if (pointer == NULL) {
		printf("Error: Out of memory\n");
		exit(1);
	}
} // }}}

void sigException(int errValue){
	if(errValue == -1){
		perror("Fail sigAction!\n");
		sleep(3);
		return;
	}
	else return;
}

void termException(int errValue){
	if(errValue == -1){
		perror("Fail termSetting!\n");
		sleep(3);
		return;
	}
	else return;
}
void initGame(TetrisGame *game){
	dieIfOutOfMemory(game);
	game->width = 10;
	game->height = 20;
	game->size = (game->width) * (game->height);
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
	termException(tcgetattr(STDIN_FILENO, &game->termOrig));
	termException(tcgetattr(STDIN_FILENO, &term));
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 0;
	termException(tcsetattr(STDIN_FILENO, TCSANOW, &term));
}
void initSig(){
	struct sigaction signalAction;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_handler = signalHandler;
	signalAction.sa_flags = 0;
	sigException(sigaction(SIGINT,  &signalAction, NULL));
	sigException(sigaction(SIGTERM, &signalAction, NULL));
	sigException(sigaction(SIGSEGV, &signalAction, NULL));
	sigException(sigaction(SIGALRM, &signalAction, NULL));
}
void initTimer(TetrisGame *game){
	game->timer.it_value.tv_usec = game->sleepUsec;
	setitimer(ITIMER_REAL, &game->timer, NULL);
}
void signalHandler(int signal) { // {{{
	switch(signal) {
		case SIGINT:
		case SIGTERM:
		case SIGSEGV:
			g_game->isRunning = 0;
			break;
		case SIGALRM:
			tick(g_game);
			g_game->timer.it_value.tv_usec = g_game->sleepUsec;
			setitimer(ITIMER_REAL, &g_game->timer, NULL);
			break;
	}
	return;
} // }}}