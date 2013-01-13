#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <term.h>
#define WIDTH        80
#define HEIGHT       24
#define MAX_LENGTH   1000
#define B_BIKE      'X'
#define B_BIKE_HEAD 'O'
#define LENGHT_LIMIT 1000
#define BULLET_SPEED 2
#define BULLET_CD 20
 
typedef struct BikeBlock_ {
    int x;
    int y;
} BikeBlock;
 
void init_game(void);
void terminate_game(void);
void append_head(int **Field, int x, int y, BikeBlock bike[], int *running);
void move_back_blocks(int **Field, BikeBlock bike[], int length);
void draw_map(int fx, int fy, BikeBlock bike[], int length);
void place_food(int *x, int *y, BikeBlock bike[], int length);
void grow(int **Field, BikeBlock bike[], int length);
void remove_tail(int **Field, BikeBlock bike[], int length);
void shoot();
void move_bullets();
_Bool is_empty(int x, int y, BikeBlock bike[], int length);
 
int main(void)
{
    int **Field;
    BikeBlock *bike;
    bike = (BikeBlock*) malloc((LENGHT_LIMIT+2)*sizeof(BikeBlock));
    WINDOW *key_detecter;
    struct timespec sleep_time;
    int i, k, j, BCD = 0;
    int xadd, yadd;
    int running;
    int length;
    int c;
    int fx, fy;
    Field = (int**)malloc(WIDTH * sizeof(int*));
    for (k = 0; k < WIDTH; k++) 
    {
	Field[k] = (int*)malloc(HEIGHT * sizeof(int));
    }
    for (k = 0; k < WIDTH; k++)
	for (j = 0; j < HEIGHT; j++)
	    Field[k][j] = 0;


    sleep_time.tv_sec  = 0;
    sleep_time.tv_nsec = 45000000;
     
    init_game();
     
    key_detecter = newwin(HEIGHT, WIDTH, 0, 0);
    nodelay(key_detecter, TRUE);
     
    running = 1;
    xadd = 0;
    yadd = 1;
     
    bike[0].x = WIDTH / 2;
    bike[0].y = HEIGHT / 2;
    //Field[bike[0].x][bike[0].y]=1;
    length = 2;
     
    while(running) {
        c = wgetch(key_detecter);
        switch (c) {
            case 'w':
                yadd = -1;
		xadd = 0;
                break;
            case 's':
                yadd = 1;
		xadd = 0;
                break;
            case 'a':
                xadd = -1;
		yadd = 0;
                break;
            case 'd':
                xadd = 1;
		yadd = 0;
                break;
	    case ' ':
		running = 2;
		BCD = BULLET_CD;
		break;
            case 'q':
                running = 0;
                break;
        }
	if (BCD > 0) BCD--;
	else
	    if (running == 2)
	    {
		
		running = 1;
	    }	

	if (bike[0].x + xadd < 0 || bike[0].x + xadd > WIDTH - 1|| bike[0].y + yadd < 0 || bike[0].y + yadd > HEIGHT)
            running = 0;
	else
	{
	    if (length<LENGHT_LIMIT+2)
		grow(Field, bike, length++);
	    else
		remove_tail(Field, bike, length);
            move_back_blocks(Field, bike, length);
            append_head(Field, xadd, yadd, bike, &running);
            draw_map(fx, fy, bike, length);
            refresh();
            nanosleep(&sleep_time, NULL); 
	}
    }
    terminate_game();
     
    for (k = 0; k < WIDTH; k++) {
	free(Field[k]);
    }
    free(Field);

    return 0;
}
 
void init_game(void)
{
    initscr();
    noecho();
    curs_set(0);
}
 
void terminate_game(void)
{
    curs_set(1);
    clear();
    endwin();
}
 
void append_head(int **Field, int x, int y, BikeBlock bike[], int *running)
{
    bike[0].x += x;
    bike[0].y += y;
    if (Field[bike[0].x][bike[0].y] == 1)
        *running = 0;
    else
	Field[bike[0].x][bike[0].y] = 1;
}
 
void move_back_blocks(int **Field, BikeBlock bike[], int length)
{
    int i;
    BikeBlock temp;
    Field[bike[length - 2].x][bike[length - 2].y] = 0;
    for (i = length; i > 0; i--) {
        temp = bike[i - 1];
        bike[i] = temp;init_game();
    }
}
 
void draw_map(int fx, int fy, BikeBlock bike[], int length)
{
    int i;
    mvaddch(bike[0].y, bike[0].x, B_BIKE_HEAD);
    mvaddch(bike[1].y, bike[1].x, B_BIKE_HEAD);
    for (i = 2; i < length; i++) {
        if (bike[i].y != -1 && bike[i].x != -1) {
            mvaddch(bike[i].y, bike[i].x, B_BIKE);
        }
    }
}
 
_Bool is_empty(int x, int y, BikeBlock bike[], int length)
{
    int i;
     
    for (i = 0; i < length; i++) {
        if (bike[i].x == x && bike[i].y == y) {
            return FALSE;
        }
    }
     
    return TRUE;
}
 
int nrand(int lower, int upper)
{
    int dif = upper - lower;
     
    srand((unsigned int) time((time_t *) NULL));
    return (int)((double)rand() / ((double)RAND_MAX + 1) * dif);
}
 
void grow(int **Field, BikeBlock bike[], int length)
{
    bike[length].x = bike[length - 1].x;
    bike[length].y = bike[length - 1].y;
    Field[bike[length].x][bike[length].y]=1;
}

void remove_tail(int **Field, BikeBlock bike[], int length)
{
    mvaddch(bike[length - 1].y, bike[length - 1].x, ' ');
    Field[bike[length - 1].x][bike[length - 1].y] = 0;
    bike[length - 1].x = -1;
    bike[length - 1].y = -1;
}

