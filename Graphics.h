#ifndef _MOVING_H_
#define _MOVING_H_

#include <ncurses.h>
#include "World.h"
#include <stdio.h>
#include "logger.h"

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
   // noecho();
   // curs_set(0);
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
	WorldCell_t tempCell;
	//TODO add borders PLEASE!
	for ( x = 0; x < MyWorld->settings.width; x++ ) {
		for ( y = 0; y < MyWorld->settings.height; y++ ) {
			tempCell = MyWorld->Field[x][y];
			if ( tempCell.type != EMPTY ) {
				
				switch ( tempCell.type ) {
					case 0: /* player */
						NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
						mvaddch(x,y,DRAW_BIKE);
						break;
					case 1: /* bullet */
						NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
						mvaddch(x,y,DRAW_BULT);
						break;
					case 2: /* tail */
						NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
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
