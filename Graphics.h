#ifndef _MOVING_H_
#define _MOVING_H_

#include <ncurses.h>
#include "World.h"

#define WIDTH        80
#define HEIGHT       24

typedef enum {
	DRAW_BIKE='X',
	DRAW_TAIL='O',
	DRAW_BULT='@',
	DRAW_BLNK=' '
}draw_t;

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

void drawWorld(World_t *MyWorld) {
	uint32_t x, y;
	//TODO add borders PLEASE!
	for ( x = 0; x < MyWorld->width; x++ ) {
		for ( x = 0; x < MyWorld->height; x++ ) {
			if ( ((WorldCell_t *)(MyWorld->Field[x][y])) != NULL ) {
				switch ( ((WorldCell_t  *)(&MyWorld->Field[x][y]))->type ) {
					case 0: /* player */
						mvaddch(x,y,DRAW_BIKE);
						break;
					case 1: /* bullet */
						mvaddch(x,y,DRAW_BULT);
						break;
					case 2: /* tail */
						mvaddch(x,y,DRAW_TAIL);
						break;
				}
			} else {
				mvaddch(x,y,DRAW_BLNK);
			}
		}
	}
}

#endif /* _MOVING_H_ */
