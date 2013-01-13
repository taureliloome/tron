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
	EMPTY=0,
	HEAD,
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

typedef struct Tail{
	uint32_t length; //Vienas astes tekošais garums;
	uint32_t playerId;  //saimnieka ID;
	struct UpdateTail* cells; //massivs ar astes šūnām;  ja aste dead -1,-1
	
}Tail_t;

typedef struct World {
	void ***Field;	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
	void *Players;		/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	void *Bullets;		/* Visu iespējamo ložu pūls. */

	void *Bikes;		/* Spēlētāju motocikli */
	Tail_t *Tails;		/* Spēlētāju astes */

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
	uint32_t tailCountAlive;
}World_t;

struct UpdatePlayer* getSelf(World_t *MyWorld)
{
	return MyWorld->Players;   // Jo massiva [0] elements ir vienads ar pointera adresi
}


void * getUpdateMessage(World_t *someWorld, size_t *length)
{
	void * packet,*tailPacket;
	struct Header  header;
	struct UpdatePlayerHeader plHeader;
	struct UpdatePlayer *tempPlayer, *playerList;

	struct UpdateBulletHeader blHeader;
	struct UpdateBullet *tempBullet,*bulletList;
	
	struct UpdateTotalTailHeader totalTailHeader;  //Visu astes galvene
	struct UpdateTailHeader *tempTailHeader; //Vienas astes galvene
	struct UpdateTail tempCell,*tempCellList; // veina astes šūna;  šūnu saraksts
	Tail_t *tempTail;  // lai ielasītu no pasaules vieni asti

	int i=0,k=0,cellLengthSumm=0,size=0;

	/* Creating packet with list of players */

	plHeader.playerCount = someWorld->playerCountAlive;
	size += sizeof(&plHeader);	

	playerList = ( struct UpdatePlayer* ) malloc( someWorld->playerCountAlive * sizeof( struct UpdatePlayer ) ); // Izdalam atminu speletaju sarakstam
	size += someWorld->playerCountAlive * sizeof( struct UpdatePlayer );	

	tempPlayer = (struct UpdatePlayer* ) someWorld->Players;

	for(i = 0; i < someWorld->playerCountAlive; i++)
	{
		memcpy(playerList, tempPlayer, sizeof(tempPlayer));  // ierakstam sarakstaa pa speletajam
		playerList += sizeof( struct UpdatePlayer);
		tempPlayer += sizeof( struct UpdatePlayer);
	}
	
	/* Creating packet with list of bullets */

	blHeader.bulletCount = someWorld->bulletCountAlive;
	size += sizeof(&blHeader);

	bulletList = (struct UpdateBullet* ) malloc( someWorld->bulletCountAlive * sizeof( struct UpdateBullet)); // Izdalam atminu lozu sarakstam
	size += someWorld->bulletCountAlive * sizeof( struct UpdateBullet );

	tempBullet = (struct UpdateBullet* ) someWorld->Bullets;

	for(i = 0; i < someWorld->bulletCountAlive; i++)
	{
		memcpy( bulletList, tempBullet, sizeof( tempBullet ) );  // ierakstam sarakstaa pa lodei
		bulletList += sizeof( struct UpdateBullet );
		tempBullet += sizeof( struct UpdateBullet );
	}
	
	/* Creating packet with list of tail */
	
	totalTailHeader.totalTailLength = someWorld->tailCountAlive;	
	tempTail = &someWorld->Tails[0];

	for(i = 0; i < someWorld->tailCountAlive; i++)
	{
		cellLengthSumm += someWorld->Tails[i].length;
	}

	tailPacket = malloc( sizeof(struct UpdateTail) * cellLengthSumm + sizeof(struct UpdateTailHeader) * someWorld->tailCountAlive );
	size += sizeof( struct UpdateTail ) * cellLengthSumm + sizeof( struct UpdateTailHeader) * someWorld->tailCountAlive;
	for(i = 0; i < someWorld->tailCountAlive; i++)
	{
		tempTailHeader = (struct UpdateTailHeader *) malloc( sizeof( struct UpdateTailHeader) ); // Izdalam atminu astes galvenei
		tempTailHeader->id = tempTail->playerId;
		tempTailHeader->tailCount = tempTail->length;
		
		
		tempCellList = (struct UpdateTail *) malloc( tempTail->length * sizeof( struct UpdateTail) );
		for(k=0 ; k < tempTail->length ; k++)
		{
			memcpy(tempCellList, &tempTail->cells[k], sizeof(tempTail->cells[k]));  // ierakstam sarakstaa pa astes šunai
			tempCellList += sizeof(struct UpdateTail);
		}
		tempTail += sizeof(Tail_t);
		memcpy(tailPacket, tempTailHeader, sizeof(tempTailHeader));		
		tailPacket += sizeof(struct UpdateTailHeader);
		memcpy(tailPacket, tempCellList, sizeof(tempCellList) );
		tailPacket += sizeof(tempCellList);
	}

	/* Calculating length of final packet */

	packet = malloc( size );
	
	memcpy(packet, &plHeader, sizeof( &plHeader) );
	packet+=sizeof(&plHeader);

	memcpy( packet, playerList, sizeof(playerList) );
	packet += sizeof( playerList );
	
	memcpy( packet, &blHeader, sizeof(&blHeader) );
	packet += sizeof(&blHeader);

	memcpy( packet , bulletList, sizeof(bulletList) );
	packet += sizeof( bulletList );
	
	memcpy( packet , &totalTailHeader, sizeof(&totalTailHeader) );
	packet += sizeof(&totalTailHeader);

	memcpy(packet, tailPacket, sizeof(tailPacket) );
	size += sizeof( tailPacket );

	*length = sizeof( packet);
	return packet;
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

