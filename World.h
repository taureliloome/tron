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
	upd_tail_t* cells; //massivs ar astes šūnām;  ja aste dead -1,-1
	
}Tail_t;

typedef struct World {
	WorldCell_t Field[24][80];	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
	void *Players;		/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	void *Bullets;		/* Visu iespējamo ložu pūls. */

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

upd_player_t* getSelf(World_t *MyWorld)
{
	return MyWorld->Players;   // Jo massiva [0] elements ir vienads ar pointera adresi
}

void init_world(World_t *someWorld)
{
	int bulletMultiplier = 0, maxSide = 0, x = 0, y = 0;
	
//TODO: remove hardcoded params
	someWorld->height = 24;    
    	someWorld->width = 80;

	if (someWorld->height >= someWorld->width)
		maxSide = someWorld->height;
	else
		maxSide = someWorld->width;

	/* Pasaules konfigurācija */

	someWorld->timeout = 100;
	someWorld->frameRate = 2;
	someWorld->bulletSpeed = 4;
	someWorld->bulletCoolDown = 3;
	someWorld->playerCountMax = 4;
	someWorld->tailLengthMax = 10;
	bulletMultiplier = maxSide / (someWorld->bulletCoolDown * someWorld->bulletSpeed);
	if (maxSide % (someWorld->bulletCoolDown * someWorld->bulletSpeed)>0)
		bulletMultiplier++;

	someWorld->bulletCountMax = bulletMultiplier * someWorld->playerCountMax;
	someWorld->bulletCountAlive = 0;
	someWorld->playerCountAlive = 0;
	someWorld->tailCountAlive = 0;

	setClientCounter(&someWorld->playerCountAlive);

	Tail_t *Tails;		/* Spēlētāju astes */

	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
#if 0 
	someWorld->Field = (WorldCell_t ***)( malloc(someWorld->width * sizeof(WorldCell_t *))); 
	for ( i= 0; i < someWorld->width; i++) {
	  someWorld->Field[i] = (WorldCell_t **) malloc( someWorld->height * sizeof(WorldCell_t));
	}
#endif

	/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	someWorld->Players =  malloc( someWorld->playerCountMax * sizeof(upd_player_t)); 

	/* Visu iespējamo ložu pūls. */
	someWorld->Bullets =  malloc( someWorld->bulletCountMax * sizeof(struct UpdateBullet) ); 
	
	/* Visu astes saraksts */
	someWorld->Tails = (Tail_t *) malloc( someWorld->playerCountMax * sizeof(Tail_t) ); 
	
}

void * getUpdateMessage(World_t *someWorld, size_t *length)
{
	if ( someWorld == NULL ) {
		*length = 0;
		return NULL;
	}
		
	void *packet,*tailPacket;

	upd_player_header_t plHeader; //speletaju galvene
	upd_player_t *tempPlayer, *playerList; // speletajs no pasaules;  speletaju saraksts aizsutisanai

	upd_bullet_header_t blHeader;  //lozu galvene
	upd_bullet_t *tempBullet,*bulletList;; // lode no pasaules;  lozu saraksts aizsutisanai
	
	upd_total_tail_header_t totalTailHeader;  //Visu astes galvene
	upd_tail_header_t *tempTailHeader; //Vienas astes galvene
	upd_tail_t tempCell,*tempCellList; // veina astes šūna;  šūnu saraksts
	Tail_t *tempTail;  // lai ielasītu no pasaules vieni asti

	int i=0, k=0, cellLengthSumm=0, size=0;  // cellLengthSumm - visu astu šunu summa, lai aprekinatu astes paketes garumu;
											// size - domats lai aprekinatu izmeru galejam paketam; 

	/* Creating packet with list of players */

	plHeader.playerCount = someWorld->playerCountAlive;
	size += sizeof(&plHeader);	

	size_t alloc_size = someWorld->playerCountAlive * sizeof(upd_player_t);
	if ( alloc_size > sizeof(upd_player_t) )
	{
		playerList = ( upd_player_t* ) malloc( alloc_size ); // Izdalam atminu speletaju sarakstam
		size += alloc_size;	
	}

	tempPlayer = (upd_player_t* ) someWorld->Players;

	for(i = 0; i < someWorld->playerCountAlive; i++)
	{
		memcpy(playerList, tempPlayer, sizeof(tempPlayer));  // ierakstam sarakstaa pa speletajam
		playerList += sizeof( upd_player_t);
		tempPlayer += sizeof( upd_player_t);
	}
	
	/* Creating packet with list of bullets */

	blHeader.bulletCount = someWorld->bulletCountAlive;
	size += sizeof(&blHeader);

	alloc_size = someWorld->bulletCountAlive * sizeof(upd_bullet_t);
	if ( alloc_size >= sizeof(upd_bullet_t) )
	{
		bulletList = (upd_bullet_t* ) malloc(alloc_size); // Izdalam atminu lozu sarakstam
		size += alloc_size;
	}
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

	/* Tail packet to store header and list of cells of each tail */

	alloc_size = sizeof(upd_tail_t) * cellLengthSumm + sizeof(upd_tail_header_t) * someWorld->tailCountAlive;
	if ( alloc_size >= sizeof(upd_tail_t) * cellLengthSumm + sizeof(upd_tail_header_t) * someWorld->tailCountAlive )
	{
		tailPacket = malloc(alloc_size);
		size += alloc_size;
	}

	for(i = 0; i < someWorld->tailCountAlive; i++)
	{
		tempTailHeader = (upd_tail_header_t *) malloc( sizeof( upd_tail_header_t) ); // Izdalam atminu astes galvenei
		tempTailHeader->id = tempTail->playerId;
		tempTailHeader->tailCount = tempTail->length;
		
		tempCellList = (upd_tail_t *) malloc( tempTail->length * sizeof( upd_tail_t) );
		for(k=0 ; k < tempTail->length ; k++)
		{
			memcpy(tempCellList, &tempTail->cells[k], sizeof(tempTail->cells[k]));  // ierakstam sarakstaa pa astes šunai
			tempCellList += sizeof(upd_tail_t);
		}
		tempTail += sizeof(Tail_t);
		memcpy(tailPacket, tempTailHeader, sizeof(tempTailHeader));		
		tailPacket += sizeof(upd_tail_header_t);
		memcpy(tailPacket, tempCellList, sizeof(tempCellList) );
		tailPacket += sizeof(tempCellList);
	}

	/* Calculating length of final packet */

	packet = malloc( size );
	
	memcpy(packet, &plHeader, sizeof( &plHeader) );
	packet+=sizeof(&plHeader);

	if ( playerList ) {
		memcpy( packet, playerList, sizeof(playerList) );
		packet += sizeof( playerList );
	}
	
	if ( playerList ) {
		memcpy( packet, &blHeader, sizeof(&blHeader) );
		packet += sizeof(&blHeader);
	}

	if ( playerList ) {
		memcpy( packet , bulletList, sizeof(bulletList) );
		packet += sizeof( bulletList );
	}
	
	if ( playerList ) {
		memcpy( packet , &totalTailHeader, sizeof(&totalTailHeader) );
		packet += sizeof(&totalTailHeader);
	}

	if ( playerList ) {
		memcpy(packet, tailPacket, sizeof(tailPacket) );
		size += sizeof( tailPacket );
	}

	*length = sizeof( packet);
	return packet;
}

void CreateClientWorld(World_t *someWorld,struct ConnectionResponse * Params)
{
	int k,i;
	someWorld->height=Params->height;
	someWorld->width=Params->width;
// WARNING SHIT CODED 2d ARRAY REFACTOR NAHOOOJ
	//someWorld->Field = malloc(Params->width * sizeof(struct WorldCell));

    //for (k = 0; k < Params->height; k++) 
    //{
	//	someWorld->Field[k] = malloc(sizeof(struct WorldCell*));
	//	memset(someWorld->Field[k], 0, sizeof(struct WorldCell*));
   // }
	someWorld->Players = (upd_player_t*)malloc(someWorld->playerCountMax * sizeof(upd_player_t));
//	for (i = 0; i < Params->width; i++)
//		for ( k = 0; k < Params->height; k++)
//		{
//			((WorldCell_t *)someWorld->Field[i][k])->x = i;
//			((WorldCell_t *)someWorld->Field[i][k])->y = K;
//			((WorldCell_t *)someWorld->Field[i][k])->type = type;
//			((WorldCell_t *)someWorld->Field[i][k])->id = id;
//			((WorldCell_t *)someWorld->Field[i][k])->dir = dir;
//		}
}

//voit InitField

void ClientMove(int c, World_t *MyWorld)
{
	upd_player_t *CurPlayer;
	CurPlayer = getSelf(MyWorld);
	switch (c) 
	{
		case 'w':
			CurPlayer->direction = DIR_UP;
			break;
		case 's':
			CurPlayer->direction = DIR_DOWN;
			break;
		case 'a':
			CurPlayer->direction = DIR_LEFT;
			break;
		case 'd':
			CurPlayer->direction = DIR_RIGHT;
			break;
		case 'q':
			//izdomaat par quit
			break;
	}
}

int getx(uint32_t dir)
{
	if (dir==DIR_UP || dir==DIR_DOWN)
		return 0;
	if (dir==DIR_LEFT)
		return -1;
	if (dir==DIR_RIGHT)
		return 1;
	return 0;
}

int gety(uint32_t dir)
{
	if (dir==DIR_LEFT || dir==DIR_RIGHT)
		return 0;
	if (dir==DIR_LEFT)
		return -1;
	if (dir==DIR_RIGHT)
		return 1;
	return 0;
}

void MoveBullets(World_t *MyWorld)
{
	int i, j;
	struct UpdateBullet *Bullets;
	Bullets = MyWorld->Bullets;
	for (i=0;i<MyWorld->bulletCountMax;i++)
	{
		if (Bullets[i].x != -1 || Bullets[i].y != -1)
			for (j=0;j<MyWorld->bulletSpeed;j++)
			{	
/*
				if (MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
					[Bullets[i].y+gety(Bullets[i].direction)]).type == BULLET)
				{
				
				}
				else
				if ((MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
					[Bullets[i].y+gety(Bullets[i].direction)]).type == TAIL)
				{

				}
				else
				if ((MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
					[Bullets[i].y+gety(Bullets[i].direction)]).type == HEAD)
				{

				}
*/
			}
	}	
}

void DeletePlayer(World_t *MyWorld, int ID)
{
	int i;
	upd_player_t *DelPlayer;
	DelPlayer = getSelf(MyWorld);
	for (i=0;i<MyWorld->playerCountMax;i++)
	{
		if (DelPlayer[i].id == ID)
		{
			DelPlayer[i].gameover=1;
			//free(MyWorld->Field[DelPlayer[i].x][DelPlayer[i].y]);
			//MyWorld->Field[DelPlayer[i].x][DelPlayer[i].y] = NULL;
			break;
		}
	}
}

void MovePlayers(World_t *MyWorld)
{
	upd_player_t *CurPlayers;
	CurPlayers = getSelf(MyWorld);
	int i;
	for (i=0;i<MyWorld->playerCountMax;i++)
	{
		if (CurPlayers[i].gameover == 0)
		{
/*
			if (MyWorld->Field[CurPlayers[i].x+getx(CurPlayers[i].direction)][CurPlayers[i].y+gety(CurPlayers[i].direction)] == NULL)
			{
				CurPlayers[i].x+=getx(CurPlayers[i].direction);
				CurPlayers[i].y+=gety(CurPlayers[i].direction);
			}
			else
			{
				DeletePlayer(MyWorld, CurPlayers[i].id);
			}
*/
		}
	}	
}

#if 0
void CreateServerWorld(World *someWorld)
{
	int maxSide = 0, bulletMultiplier = 0;
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
	bulletMultiplier = maxSide/(MyWorld->bulletCoolDown*MyWorld->bulletSpeed);
	if (maxSide%(MyWorld->bulletCoolDown*MyWorld->bulletSpeed)>0)
		bulletMultiplier++;
	MyWorld->bulletCount=bulletMultiplier*MyWorld->playerCount;
	MyWorld->Field=(WorldCell**)malloc(MyWorld->width * sizeof(WorldCell*));
	MyWorld->Players= (UpdatePlayer*)malloc(MyWorld->playerCount * sizeof(UpdatePlayer*));
	MyWorld->Bullets=(UpdateBullet*)malloc(bulletMultiplier * sizeof(UpdateBullet*));
}

void addNewPlayer(int ID)
{
	
}
void CreateClientWorld(World *someWorld,ConnectionResponse * Params)
{
	int maxSide = 0, bulletMultiplier = 0;
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
		maxSide = MyWorld->height;
	else
		maxSide = MyWorld->width;
	MyWorld->tailLength=Params->tailLength
	MyWorld->frameRate=Params->frameRate;
	MyWorld->bulletSpeed=Params->bulletSpeed;
	MyWorld->bulletCoolDown=Params->bulletCoolDown;
	MyWorld->timeout=Params->timeout;	
	MyWorld->playerCount=Params->playerCount;
	bulletMultiplier = maxSide/(MyWorld->bulletCoolDown*MyWorld->bulletSpeed);
	if (maxSide%(MyWorld->bulletCoolDown*MyWorld->bulletSpeed)>0)
		bulletMultiplier++;
	MyWorld->bulletCount=bulletMultiplier*MyWorld->playerCount;
	MyWorld->Bullets=(UpdateBullet*)malloc(bulletMultiplier * sizeof(UpdateBullet*));
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
	upd_player_header_t * plHeader=Params;
	upd_player_t * player;	
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

