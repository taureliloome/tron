#ifndef _MOVING_H_
#define _MOVING_H_

#include "World.h"

#define WIDTH        80
#define HEIGHT       24

static WINDOW *key_detecter;
static struct timespec sleep_time;
void init_game(void)
{
    initscr();
    noecho();
    curs_set(0);
	key_detecter = newwin(HEIGHT, WIDTH, 0, 0);
    nodelay(key_detecter, TRUE);
};

void terminate_game(void)
{
    curs_set(1);
    clear();
    endwin();
};


#endif /* _MOVING_H_ */
