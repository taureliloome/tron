#ifndef _WORLD_H_
#define _WORLD_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <term.h>
#include <stdarg.h>
#include <stdint.h>
#include "packets.h"



/* Object type enumerator */
typedef enum Objects{
	HEAD=1,
	BACK,
	TAIL,
	BULLET
}object_t;

/* One cell structure */
typedef struct WorldCell {
	uint32_t x;
	uint32_t y;
    uint32_t type;      //Šūnas tips
    uint32_t id;	//Spēlētaja id, kuram pieder objekts 
	uint32_t dir;
}__attribute__((packed)) WorldCell_t ;

typedef struct World {
	void ***Field;	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
	void *Players;		/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	void *Bullets;		/* Visu iespējamo ložu pūls. */

	void *Bikes;		/* Spēlētāju motocikli */
	void **Tails;		/* Spēlētāju astes */

	/* Pasaules configurācija */
   	uint32_t height;    
    uint32_t width;
	uint32_t timeout;
	uint32_t frameRate;
	uint32_t bulletSpeed;
	uint32_t bulletCoolDown;
	uint32_t playerCountMax;
	uint32_t tailLengthMax;

	uint32_t bulletCountMax;
	uint32_t bulletCountAlive;
	uint32_t playerCountAlive;
}World_t;

struct UpdatePlayer* getSelf(World_t *MyWorld)
{
	return MyWorld->Players;
}
#if 0
void CreateServerWorld(World *someWorld)
{
	int MaxSide = 0, BulletMultiplier = 0;
	MyWorld=someWorld;
	
	MyWorld->Bikes;
    MyWorld->height=80;           
    MyWorld->width=20;            
	MyWorld->tailLength=10;
	MyWorld->frameRate=60;
	MyWorld->bulletSpeed=2;
	MyWorld->bulletCoolDown=20;
	MyWorld->bulletCountAlive=0;
	MyWorld->timeout=100;
	MyWorld->playerCount=4;
	MyWorld->playerCountAlive=0;
	BulletMultiplier = MaxSide/(MyWorld->bulletCoolDown*MyWorld->bulletSpeed);
	if (MaxSide%(MyWorld->bulletCoolDown*MyWorld->bulletSpeed)>0)
		BulletMultiplier++;
	MyWorld->bulletCount=BulletMultiplier*MyWorld->playerCount;
	MyWorld->Field=(WorldCell**)malloc(MyWorld->width * sizeof(WorldCell*));
	MyWorld->Players= (UpdatePlayer*)malloc(MyWorld->playerCount * sizeof(UpdatePlayer*));
	MyWorld->Bullets=(UpdateBullet*)malloc(BulletMultiplier * sizeof(UpdateBullet*));
}

void addNewPlayer(int ID)
{
	
}
void CreateClientWorld(World *someWorld,ConnectionResponse * Params)
{
	int MaxSide = 0, BulletMultiplier = 0;
	MyWorld=someWorld;
	MyWorld->playerCountAlive=0;
	MyWorld->Field = (WorldCell**)malloc(Params->WIDTH * sizeof(WorldCell*));
    for (k = 0; k < Params->WIDTH; k++) 
    {
		memset(MyWorld->Field[k], 0, sizeof(WorldCell*));
    }
	MyWorld->height=Params->height;	
	MyWorld->width=Params->width;
    MyWorld->Players = (UpdatePlayer*)malloc(Params->playerCount * sizeof(UpdatePlayer*));
	if (MyWorld->height>=MyWorld->width)
		MaxSide = MyWorld->height;
	else
		MaxSide = MyWorld->width;
	MyWorld->tailLength=Params->tailLength
	MyWorld->frameRate=Params->frameRate;
	MyWorld->bulletSpeed=Params->bulletSpeed;
	MyWorld->bulletCoolDown=Params->bulletCoolDown;
	MyWorld->timeout=Params->timeout;	
	MyWorld->playerCount=Params->playerCount;
	BulletMultiplier = MaxSide/(MyWorld->bulletCoolDown*MyWorld->bulletSpeed);
	if (MaxSide%(MyWorld->bulletCoolDown*MyWorld->bulletSpeed)>0)
		BulletMultiplier++;
	MyWorld->bulletCount=BulletMultiplier*MyWorld->playerCount;
	MyWorld->Bullets=(UpdateBullet*)malloc(BulletMultiplier * sizeof(UpdateBullet*));
	for (i=0;i<MyWorld->bulletCount;i++)
		MyWorld->Bullets[i]->direction = -1;
};

void bulletFree(int PlayerID, int x, int y)
{
	int i;
	for (i=(MyWorld->bulletCount/MyWorld->playerCount)*PlayerID;i<MyWorld->bulletCount;i++)
		if (MyWorld->Bullets->id == PlayerdID && MyWorld->Bullets->x == x && MyWorld->Bullets->y == y)
		{
			MyWorld->Bullets[i]->direction = -1;
			MyWorld->bulletCountAlive--;
			free(MyWorld->Field[x][y]);
		}
}
int findPlayer(int playerID)
{
	int i=0;	
	for(i=0;i<MyWorld->playerCount;i++)
	{
		if(MyWorld->Players[i]->id==playerID) return 1;
	}	
	return 0;
}
void GetBullet(int PlayerID, int x, int y, int dir)
{
	for (i=(MyWorld->bulletCount/MyWorld->playerCount)*PlayerID;i<MyWorld->bulletCount;i++)
		if (MyWorld->Bullets[i]->direction > -1)
		{	
			MyWorld->Bullets[i]->direction = dir;
			MyWorld->Bullets[i]->x = x;
			MyWorld->Bullets[i]->y = y;
			MyWorld->Bullets[i]->ID = PlayerID;
			MyWorld->bulletCountAlive++;
			MyWorld->Field[x][y]=(WorldCell*)malloc(sizeof(WorldCell*));
			MyWorld->Field[x][y]->x = x;
			MyWorld->Field[x][y]->y = y;
			MyWorld->Field[x][y]->direction = dir;
			MyWorld->Field[x][y]->id = PlayerID;
			MyWorld->Field[x][y]->type = BULLET;
		}
}

void collision()
{
	
};

void UpdateWorld(void * Params)
{
	struct UpdatePlayerHeader * plHeader=Params;
	struct UpdatePlayer * player;	
	int i=0;	
	for(i=0;i<plHeader->playerCount;i++)
	{
		player=Params+sizeof(UpdatePlayerHeader);
		if(!findPlayer(player->id))
		{
			//TODO: CJ`s id gens goes here
			MyWorld->Players[MyWorld->playerCountAlive]->
			MyWorld->playerCountAlive++;
		}
	}
	
}

void *GetWorldStatus(size_t *len)
{
	int playercount = MyWorld->playerCount;
	int bulletcount = MyWorld->bulletCountAlive;
}

#endif

#endif /* _WORLD_H_ */

