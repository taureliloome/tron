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
				switch ( tempCell.type ) {
					case HEAD:
						mvaddch(MyWorld->settings.height-y,x,'<');
						break;
					case BACK: /* BIKE */
						//NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
						mvaddch(MyWorld->settings.height-y,x,'>');
						break;
					case TAIL: /* tail */
						//NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
						mvaddch(MyWorld->settings.height-y,x,DRAW_TAIL);
						break;
					case BULLET: /* bullet */
						//NOTICE("x: %d,y:%d, type: %d\n",x,y,tempCell.type);
						mvaddch(MyWorld->settings.height-y,x,DRAW_BULT);
						break;
					default:
						mvaddch(MyWorld->settings.height-y,x,DRAW_BLNK);
						break;					
				}
		}
	}
}

#endif /* _MOVING_H_ */
