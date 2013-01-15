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
}__attribute__((packed)) WorldCell_t;

typedef struct tail_s{
	uint32_t length; 		//Vienas astes tekošais garums;
	uint32_t playerId;  	//saimnieka ID;
	uint32_t currentId;
	upd_tail_t* cells;		//massivs ar astes šūnām;  ja aste dead -1,-1
	
}tail_t;

typedef struct world_s {
	WorldCell_t Field[80][24];	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
	upd_player_t *Players;		/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	upd_bullet_t *Bullets;		/* Visu iespējamo ložu pūls. */

	tail_t *tails;				/* Spēlētāju astes */

	conn_resp_t settings;     	/* Pasaules configurācija */

	uint32_t bulletCountMax;
	uint32_t bulletCountAlive;
	uint32_t playerCountAlive;
	uint32_t tailCountAlive;
}World_t;

void update_player_tail(World_t *someWorld, uint32_t playerId)
{
	tail_t *my_tail = &someWorld->tails[playerId];
	my_tail->cells[my_tail->currentId].x = my_tail->cells[my_tail->currentId+1].x + 0;
	my_tail->cells[my_tail->currentId].y = my_tail->cells[my_tail->currentId+1].y + 0;
	my_tail->currentId++;
	if ( my_tail->currentId >= sizeof(my_tail) / sizeof(tail_t) )
		my_tail->currentId = 0;
}

upd_player_t* getSelf(World_t *MyWorld)
{
	return MyWorld->Players;   // Jo massiva [0] elements ir vienads ar pointera adresi
}

tail_t * createTail(World_t *someWorld, int id)
{
	int tailCount = someWorld->tailCountAlive;
	if(someWorld->tailCountAlive < someWorld->playerCountAlive){

		int i = 0; 
		someWorld->tails[tailCount].playerId = id;
		someWorld->tails[tailCount].length = 0;
		someWorld->tails[tailCount].cells = malloc( sizeof(upd_tail_t) * someWorld->settings.tailLength);

		for(i; i < someWorld->settings.tailLength; i++)
			someWorld->tails[tailCount].cells[i].x = someWorld->tails[tailCount].cells[i].y = -1;

		someWorld->tailCountAlive++;
		return &someWorld->tails[tailCount];
	}
	return NULL;
}

upd_player_t * createPlayer(World_t *someWorld, int id,int x,int y)
{
	int pc = someWorld->playerCountAlive;
	if(someWorld->playerCountAlive < someWorld->settings.playerCount){
		
		someWorld->Players[pc].id = id;
		someWorld->Players[pc].x = x;
		someWorld->Players[pc].y = y;
		someWorld->Players[pc].direction = DIR_UP;
		someWorld->Players[pc].cooldown = 0;         // Lodes cooldown (Kadros)
	    	someWorld->Players[pc].gameover = 0;         // 1 - miris, 0 - spēlē

		someWorld->playerCountAlive++;

		return &someWorld->Players[pc] ;
	}else{
	  return NULL;
	}
}

upd_bullet_t *getFreeBullet(World_t * someWorld)
{
	upd_bullet_t * bullet;
	int i = 0;
	for( i; i < someWorld->bulletCountMax; i++){
		if(someWorld->Bullets[i].x > -1 && someWorld->Bullets[i].y > -1){
			someWorld->bulletCountAlive++;
			return &someWorld->Bullets[i];
		}		
	}
	return NULL;
}

upd_player_t * findPlayer(World_t *someWorld, int playerId)
{
	int i = 0;
	upd_player_t pl;
	for(i = 0 ; i < someWorld->playerCountAlive; i++)
	{
		pl = someWorld->Players[i];
		if(pl.id == playerId)
		{
			return &someWorld->Players[i];
		}
	}
	return NULL;
}

upd_bullet_t * findBullet(World_t *someWorld, int bulletId)
{
	int i = 0;
	upd_bullet_t bl;
	for(i = 0 ; i < someWorld->bulletCountAlive; i++)
	{
		bl = someWorld->Bullets[i];
		if(bl.id == bulletId)
		{
			return &someWorld->Bullets[i];
		}
	}
	return NULL;
}

tail_t * findtail(World_t *someWorld , int tailId)
{
	int i = 0;
	tail_t tl;
	for(i = 0 ; i < someWorld->tailCountAlive; i++)
	{
		tl = someWorld->tails[i];
		if(tl.playerId == tailId)
		{
			return &someWorld->tails[i];
		}
	}
	return NULL;
}

void updateClientWorld(World_t *someWorld,void * packet)
{
	upd_player_header_t *playerHeader;
	upd_player_t *tempPlayer, *existPlayer;

	upd_bullet_header_t *bulletHeader;
	upd_bullet_t *tempBullet, *existBullet;

	upd_total_tail_header_t *tailHeader;
	upd_tail_header_t *temptail;
	upd_tail_t *tempCell;
	tail_t *existtail;

	void *iterator = packet;
	int i = 0, k = 0, t = 0;


	DEBUG2("Decoding Update Message\n");
	/* Handling player list */
	playerHeader = (upd_player_header_t *) iterator;
	iterator += sizeof(upd_player_header_t);
	DEBUG2("\tHeader playerHeader->playerCount: %u\n", playerHeader->playerCount );

	for(i = 0 ; i < playerHeader->playerCount; i++){
		
		tempPlayer = (upd_player_t *) iterator;

		existPlayer = findPlayer(someWorld, tempPlayer->id); //meklejam speletaju sarakstaa
		if(existPlayer == NULL){
			existPlayer = createPlayer( someWorld, tempPlayer->id, tempPlayer->x, tempPlayer->y); //izveidojam speletaju ja nav
		}

		if(existPlayer != NULL){ //pat ja tads speletajs nepastaveja,  var but ka na brivo vietu
			memcpy(existPlayer, tempPlayer, sizeof(upd_player_t) ); //kopejam izmainas
		}

		iterator += sizeof(upd_player_t);
	}

	DEBUG2("\tIterations used: %d \n", i);

	/* Handling Bullet list */

	bulletHeader = (upd_bullet_header_t *) iterator;
	iterator += sizeof(upd_bullet_header_t);
	

	DEBUG2("\tHeader bulletHeader->bulletCount: %u\n", bulletHeader->bulletCount );

	for( i = 0; i < bulletHeader->bulletCount; i++){
		
		tempBullet = (upd_bullet_t *) iterator;
		existBullet = findBullet(someWorld, tempBullet->id); //meklejam lodi sarakstaa
		
		if(existBullet == NULL){
			existBullet = getFreeBullet( someWorld); // atgriez brivo lodi no pula vai NULL		
		}
		
		if(existBullet != NULL){
			memcpy(existBullet,tempBullet, sizeof(upd_bullet_t) );
		}
	
		iterator += sizeof(upd_bullet_t);
	}

	DEBUG2("\tIterations used: %d \n", i);


	/* Handling tails */

	tailHeader = (upd_total_tail_header_t*) iterator;
	iterator += sizeof(upd_total_tail_header_t);


	DEBUG2("\tHeader tailHeader->totalTailLength %u\n", tailHeader->totalTailLength );

	for( i = 0; i < tailHeader->totalTailLength; i++){
		temptail = (upd_tail_header_t*) iterator;
		iterator += sizeof(upd_tail_header_t);

		existtail = findtail(someWorld , temptail->id);	

		/*TODO: VEIDOT ASTES KOPA AR SPELETAJA VEIDOSANU*/

		if(existtail != NULL){  //visas astes ir izveidotas kad tiek veidots speletajs 
			
			existtail->length = temptail->tailCount;
			
			for( k = 0; k < temptail->tailCount; k++){
				tempCell = (upd_tail_t *) iterator;
				existtail->cells[k].x = tempCell->x;
				existtail->cells[k].y =	tempCell->y;
				iterator += sizeof(upd_tail_t);
			}
		}
		
	}

	DEBUG2("\tIterations used: %d \n", i);

}
void init_world(World_t *someWorld)
{
	int bulletMultiplier = 0, maxSide = 0, x = 0, y = 0, i = 0;
	
//TODO: remove hardcoded params
	someWorld->settings.height = 24;    
	someWorld->settings.width = 80;

	if (someWorld->settings.height >= someWorld->settings.width)
		maxSide = someWorld->settings.height;
	else
		maxSide = someWorld->settings.width;

	/* Pasaules konfigurācija */

	someWorld->settings.timeout = 100;
	someWorld->settings.frameRate = 2;
	someWorld->settings.bulletSpeed = 4;
	someWorld->settings.bulletCooldown = 3;
	someWorld->settings.playerCount = 1;
	someWorld->settings.tailLength = 10;
	bulletMultiplier = maxSide / (someWorld->settings.bulletCooldown * someWorld->settings.bulletSpeed);
	if (maxSide % (someWorld->settings.bulletCooldown * someWorld->settings.bulletSpeed)>0)
		bulletMultiplier++;

	someWorld->bulletCountMax = bulletMultiplier * someWorld->settings.playerCount;
	someWorld->bulletCountAlive = 0;
	someWorld->playerCountAlive = 0;
	someWorld->tailCountAlive = 0;

	setClientCounter(&someWorld->playerCountAlive);

	tail_t *tails;		/* Spēlētāju astes */

	/* Divdimensiju masivs ar noradem uz objektu kurš aizņem konkrētu šunu. */
#if 0 
	someWorld->Field = (WorldCell_t ***)( malloc(someWorld->width * sizeof(WorldCell_t *))); 
	for ( i= 0; i < someWorld->width; i++) {
	  someWorld->Field[i] = (WorldCell_t **) malloc( someWorld->height * sizeof(WorldCell_t));
	}
#endif

	/* Visu spēlētāju objektu pūls. Satur koordinātes, virzienu utl. */
	someWorld->Players =  malloc( someWorld->settings.playerCount * sizeof(upd_player_t)); 

	/* Visu iespējamo ložu pūls. */
	someWorld->Bullets =  malloc( someWorld->bulletCountMax * sizeof(upd_bullet_t) ); 
	for(i = 0; i < someWorld->bulletCountMax; i++){
		someWorld->Bullets[i].id = i;  //pieskiram id lodem (DOMATS SERVER PUSEI JA KAS)
		someWorld->Bullets[i].x=someWorld->Bullets[i].y=-1; //atbrivojam visas lodes
	}

	/* Visu astes saraksts */
	someWorld->tails = (tail_t *) malloc( someWorld->settings.playerCount * sizeof(tail_t) ); 
	
}

void * getUpdateMessage(World_t *someWorld, size_t *length)
{
	if ( someWorld == NULL ) {
		*length = 0;
		return NULL;
	}
		
	void *packet, *iterator, *tailPacket;

	upd_player_header_t plHeader; //speletaju galvene
	upd_player_t *tempPlayer, *playerList; // speletajs no pasaules;  speletaju saraksts aizsutisanai

	upd_bullet_header_t blHeader;  //lozu galvene
	upd_bullet_t *tempBullet,*bulletList;; // lode no pasaules;  lozu saraksts aizsutisanai
	
	upd_total_tail_header_t totaltailHeader;  //Visu astes galvene
	upd_tail_header_t *temptailHeader; //Vienas astes galvene
	upd_tail_t tempCell,*tempCellList; // veina astes šūna;  šūnu saraksts
	tail_t *temptail;  // lai ielasītu no pasaules vieni asti

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
	//DEBUG("BULLETHEADER->BULLETCOUNT: %d\n",blHeader.bulletCount);
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
	
	totaltailHeader.totalTailLength = someWorld->tailCountAlive;	
	temptail = &someWorld->tails[0];

	for(i = 0; i < someWorld->tailCountAlive; i++)
	{
		cellLengthSumm += someWorld->tails[i].length;
	}

	/* tail packet to store header and list of cells of each tail */

	alloc_size = sizeof(upd_tail_t) * cellLengthSumm + sizeof(upd_tail_header_t) * someWorld->tailCountAlive;
	if ( alloc_size >= sizeof(upd_tail_header_t) )
	{
		tailPacket = malloc(alloc_size);
		size += alloc_size;
	}

	for(i = 0; i < someWorld->tailCountAlive; i++)
	{
		temptailHeader = (upd_tail_header_t *) malloc( sizeof( upd_tail_header_t) ); // Izdalam atminu astes galvenei
		temptailHeader->id = temptail->playerId;
		temptailHeader->tailCount = temptail->length;
		
		tempCellList = (upd_tail_t *) malloc( temptail->length * sizeof( upd_tail_t) );
		for(k=0 ; k < temptail->length ; k++)
		{
			memcpy(tempCellList, &temptail->cells[k], sizeof(temptail->cells[k]));  // ierakstam sarakstaa pa astes šunai
			tempCellList += sizeof(upd_tail_t);
		}
		temptail += sizeof(tail_t);
		memcpy(tailPacket, temptailHeader, sizeof(temptailHeader));		
		tailPacket += sizeof(upd_tail_header_t);
		memcpy(tailPacket, tempCellList, sizeof(tempCellList) );
		tailPacket += sizeof(tempCellList);
	}

	/* Calculating length of final packet */

	iterator = packet = malloc( size );
	
	memcpy(iterator, &plHeader, sizeof(plHeader) );
	iterator += sizeof(&plHeader);

	if ( playerList ) {
		memcpy( iterator, playerList, sizeof(*playerList) );
		iterator += sizeof( playerList );
	}
	
	memcpy( iterator, &blHeader, sizeof(blHeader) );
	iterator += sizeof(&blHeader);

	if ( bulletList ) {
		memcpy( iterator , bulletList, sizeof(*bulletList) );
		iterator += sizeof( bulletList );
	}
	
	memcpy( iterator , &totaltailHeader, sizeof(totaltailHeader) );
	iterator += sizeof(&totaltailHeader);

	if ( tailPacket ) {
		memcpy(iterator, tailPacket, sizeof(*tailPacket) );
		iterator += sizeof( tailPacket );
	}

	*length = size;
	return packet;
}



void CreateClientWorld(World_t *someWorld,conn_resp_t * Params)
{
	int k,i;
	WorldCell_t tempCell;
	someWorld->settings = *Params;
	//someWorld->Players = (upd_player_t*)malloc(someWorld->settings.playerCount * sizeof(upd_player_t));
	
//	someWorld->Field = malloc(Params->width * Params->height * sizeof(void**));
//
//   	for (k = 0; k < Params->width; k++) 
//  	{
//			someWorld->Field[k] = malloc(Params->height * sizeof(void*));
//			for(i = 0; i < Params->height; i++)
//			{
//				tempCell = someWorld->Field[k][i];
//				tempCell.x = k;
//				tempCell.y = i;
//			}
//    	}

	someWorld->Players = malloc(someWorld->settings.playerCount * sizeof(upd_player_t));
	someWorld->Bullets = malloc(someWorld->bulletCountMax * sizeof(upd_bullet_t));
	someWorld->tails = malloc(someWorld->settings.playerCount * sizeof(tail_t));
	for(i = 0;i < someWorld->settings.playerCount; i++)
	{
		someWorld->Players[i].gameover=1;
	}
	
	for ( i = 0; i < someWorld->settings.tailLength; i++ )
	{
		someWorld->tails[i].cells = (upd_tail_t *) malloc(someWorld->settings.tailLength);
	}

	for (i = 0; i < Params->width; i++)
	{
		for ( k = 0; k < Params->height; k++)
		{
			someWorld->Field[i][k].x	= i;
			someWorld->Field[i][k].y	= k;
			someWorld->Field[i][k].type	= EMPTY;
			someWorld->Field[i][k].id	= 0;
			someWorld->Field[i][k].dir	= DIR_MAX;
		}
	}

	createPlayer(someWorld , Params->id,40,10); //izveidojam pirmo speletaju, to kuru vadis tekošais klients
	createTail(someWorld, Params->id); //izveidojam vinam asti garuma 0

}


void ClientMove(int c, World_t *MyWorld)
{
	upd_player_t *CurPlayer;
	CurPlayer = getSelf(MyWorld);
	switch (c) 
	{
		case 'w':
			if (CurPlayer->direction != DIR_DOWN)
				CurPlayer->direction = DIR_UP;
			break;
		case 's':
			if (CurPlayer->direction != DIR_UP)
				CurPlayer->direction = DIR_DOWN;
			break;
		case 'a':
			if (CurPlayer->direction != DIR_RIGHT)
				CurPlayer->direction = DIR_LEFT;
			break;
		case 'd':
			if (CurPlayer->direction != DIR_LEFT)
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
	if (dir==DIR_DOWN)
		return -1;
	if (dir==DIR_UP)
		return 1;
	return 0;
}

void setValuesCell(WorldCell_t *Cell, int type, int id, int dir)//Pieskir Field cell vertibas
{
	Cell->type = type;
	Cell->id = id;
	Cell->dir = dir;
}

void bulletHitPlayer(World_t *MyWorld, upd_player_t *Players, tail_t *Tails, int count, int x, int y)
{
	int i, j, k;
	int *tx, *ty;
	for (i=0;i<count;i++)
	{
		if (Players[i].gameover == 0)//Mekle nezaudejuso speletaju kas atrodas x, y coord
		{
			if (Players[i].x == x && Players[i].y == y)
			{
				Players[i].gameover = 1;				
				setValuesCell(&MyWorld->Field[x][y], EMPTY, -1, -1);//izdzes speletaja moca prieksas datus
				x-=getx(Players[i].direction);//Dabun moca aizmugures x coord
				y-=gety(Players[i].direction);//Dabun moca aizmugures y coord
				setValuesCell(&MyWorld->Field[x][y], EMPTY, -1, -1);//izdzes speletaja moca aizmugures datus
				for (j=0;j<count;j++)//Atrod speletaja asti
				{
					if (Tails[j].playerId == Players[i].id)
					{
						for (k=0;k<Tails[j].length;k++)//Izdzes speletaja astes datus
						{
							tx = &Tails[j].cells[k].x;
							ty = &Tails[j].cells[k].y;
							setValuesCell(&MyWorld->Field[*tx][*ty], EMPTY, -1, -1);
							*ty = -1;
							*tx = -1;
						}
						break;//Iziet no astes meklesanas cikla
					}
				}
				break;//Iziet no speletaja meklesanas cikla 
			}
		}
	}
}

bulletHitBack(World_t *MyWorld, upd_player_t *Players, tail_t *Tails, int count, int x, int y)
{
	int i, j, k;
	int *tx, *ty;
	setValuesCell(&MyWorld->Field[x][y], EMPTY, -1, -1);//izdzes speletaja moca aizmuguri datus
	x+=getx(Players[i].direction);//Dabun moca prieksas x coord
	y+=gety(Players[i].direction);//Dabun moca prieksas y coord
	for (i=0;i<count;i++)
	{
		if (Players[i].gameover == 0)//Mekle nezaudejuso speletaju kas atrodas x, y coord
		{
			if (Players[i].x == x && Players[i].y == y)
			{
				Players[i].gameover = 1;				
				setValuesCell(&MyWorld->Field[x][y], EMPTY, -1, -1);//izdzes speletaja moca prieksas datus
				for (j=0;j<count;j++)//Atrod speletaja asti
				{
					if (Tails[j].playerId == Players[i].id)
					{
						for (k=0;k<Tails[j].length;k++)//Izdzes speletaja astes datus
						{
							tx = &Tails[j].cells[k].x;
							ty = &Tails[j].cells[k].y;
							setValuesCell(&MyWorld->Field[*tx][*ty], EMPTY, -1, -1);
							*ty = -1;
							*tx = -1;
						}
						break;//Iziet no astes meklesanas cikla
					}
				}
				break;//Iziet no speletaja meklesanas cikla 
			}
		}
	}
}

HitTail(World_t *MyWorld, upd_player_t *Players, tail_t *Tails, int count, int x, int y)
{
	int j, k, i=0;
	int *tx, *ty;
	for (j=0;j<count;j++)//Atrod asti
	{
		for (k=1;k<Tails[j].length;k++)//Izdzes sasautas astes datus
		{
			if (Tails[j].cells[0].x == x && Tails[j].cells[0].y == y)
			{
				tx = &Tails[j].cells[k].x;
				ty = &Tails[j].cells[k].y;
				setValuesCell(&MyWorld->Field[*tx][*ty], EMPTY, -1, -1);
				*ty = -1;
				*tx = -1;
				i = 1;
				break;//Iziet no astes apstaigasanas cikla
			}
		}
		if (i) break;//Iziet no astes meklesanas cikla kad sasauta aste ir izdzesta
	}
}

HitBullet(World_t *MyWorld, upd_bullet_t *Bullets, int count, int x, int y)
{
	int j, k, i=0;
	int *tx, *ty;
	for (j=0;j<count;j++)
	{
		if (Bullets[j].x == x && Bullets[j].y == y)
		{
			Bullets[j].x = -1;
			Bullets[j].y = -1;
			setValuesCell(&MyWorld->Field[x][y], EMPTY, -1, -1);
			break;
		}
	}
}

void MoveBullets(World_t *MyWorld)
{
	int i, j, xd, yd, id;
	int *x, *y, dir;
	upd_bullet_t *Bullets;
	Bullets = MyWorld->Bullets;
	for (i=0;i<MyWorld->bulletCountMax;i++)//Visu lozu pakustinasana un parbaude uz kolizijam
	{
		x = &Bullets[i].x;
		y = &Bullets[i].y;
		dir = Bullets[i].direction;
		id = Bullets[i].id;
		if (*x != -1 || *y != -1)//Parbauda visas lodes
			for (j=0;j<MyWorld->settings.bulletSpeed;j++)//Parbauda visu lodes paveikto celju
			{	
				xd = *x+getx(dir);
				yd = *y+gety(dir);
				if (xd<0 || xd>MyWorld->settings.width || yd<0 || yd>MyWorld->settings.height)//Parbaude vai neiziet arpus kartes
				{
					setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
					*x = -1;
					*y = -1;
					break;
				}
				else
				switch(MyWorld->Field[xd][yd].type)
				{
					case EMPTY://Ja nekas lodei nav prieksa tad pabidam lodi uz prieksu
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						*x = xd;
						*y = yd;
						setValuesCell(&MyWorld->Field[xd][yd], BULLET, id, dir);
						break;
					case HEAD://Ja lode ietriecas moca galva tad moci un lodi izdzes
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						bulletHitPlayer(MyWorld, MyWorld->Players, MyWorld->tails, MyWorld->settings.playerCount, xd, yd);
						*x = -1;
						*y = -1;
						break;
					case BACK://Ja lode ietriecas moca aizmugure tad moci un lodi izdzes
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						bulletHitBack(MyWorld, MyWorld->Players, MyWorld->tails, MyWorld->settings.playerCount, xd, yd);
						*x = -1;
						*y = -1;
						break;
					case TAIL://Ja lode ietriecas aste tad astes gabalu un lodi izdzes
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						HitTail(MyWorld, MyWorld->Players, MyWorld->tails, MyWorld->settings.playerCount, xd, yd);
						*x = -1;
						*y = -1;
						break;
					case BULLET://Ja lode ietriecas lode tad abas lodes izdzes
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						HitBullet(MyWorld, Bullets, MyWorld->bulletCountMax, xd, yd);
						*x = -1;
						*y = -1;
						break;
				}
				
			}
			
	}	
}

void DeletePlayer(World_t *MyWorld, upd_player_t *Player, tail_t *Tails, int i, int count)
{
	int j, k;
	int *x, *y;
	x = &Player[i].x;
	y = &Player[i].y;

	Player[i].gameover = 1;				
	setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);//izdzes speletaja moca prieksas datus
	x-=getx(Player[i].direction);//Dabun moca aizmugures x coord
	y-=gety(Player[i].direction);//Dabun moca aizmugures y coord
	setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);//izdzes speletaja moca aizmugures datus
	for (j=0;j<count;j++)//Atrod speletaja asti
	{
		if (Tails[j].playerId == Player[i].id)
		{
			for (k=0;k<Tails[j].length;k++)//Izdzes speletaja astes datus
			{
				x = &Tails[j].cells[k].x;
				y = &Tails[j].cells[k].y;
				setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
				*y = -1;
				*x = -1;
			}
			break;//Iziet no astes meklesanas cikla
		}
	}
}

void DeletePlayerID(World_t *MyWorld, upd_player_t *Player, tail_t *Tails, int count, int id)
{
	int i;
	for (i=0;i<count;i++)
	{
		if (Player->gameover == 0)//Mekle nezaudejuso speletaju kas atrodas x, y coord
		{
			if (Player[i].id == id)
			{
				DeletePlayer(MyWorld, &Player[0], Tails, i, count);
				break;//Iziet no speletaju meklesanas cikla
			}
		}
	}
}

void moveTail(World_t *MyWorld, tail_t *Tails, int id, int x, int y, int count, int dir)
{
	int j, k, tx, ty;
	for (j=0;j<count;j++)//Atrod speletaja asti
	{
		if (Tails[j].playerId == id)
		{
			setValuesCell(&MyWorld->Field[x][y], TAIL, id, dir);
			for (k=0;k<Tails[j].length-1;k++)//Pabida asti uz prieksu
			{
				tx = Tails[j].cells[k].x;
				ty = Tails[j].cells[k].y;
				Tails[j].cells[k].x = x;
				Tails[j].cells[k].y = y;
				x = tx;
				y = ty;
			}
			if (Tails[j].length == MyWorld->settings.tailLength)
			{
				setValuesCell(&MyWorld->Field[Tails[j].cells[Tails[j].length].x][Tails[j].cells[Tails[j].length].y], EMPTY, -1, -1);
			}
			else
			{
				Tails[j].length++;
			}
			break;//Iziet no astes meklesanas cikla
		}
	}
}

void MovePlayers(World_t *MyWorld)
{
	int i, j, xd, yd, id, tx, ty;
	int *x, *y, dir;
	upd_player_t *CurPlayers;
	CurPlayers = getSelf(MyWorld);
	for (i=0;i<MyWorld->settings.playerCount;i++)
	{
		if (CurPlayers[i].gameover == 0)//Apstaiga visus dzivos speletajus
		{
			x = &CurPlayers[i].x;
			y = &CurPlayers[i].y;
			dir = CurPlayers[i].direction;
			xd = *x+getx(dir);
			yd = *y+gety(dir);
			tx = *x;
			ty = *y;
			if (xd<0 || xd>MyWorld->settings.width || yd<0 || yd>MyWorld->settings.height)//Parbaude vai neiziet arpus kartes
			{
				DeletePlayer(MyWorld, &CurPlayers[0], MyWorld->tails, i, MyWorld->settings.playerCount);
				break;
			}
			else
			switch(MyWorld->Field[xd][yd].type)
			{
				case EMPTY://Ja nekas mocim nav prieksa tad pabidam moci uz prieksu
					tx-=getx(MyWorld->Field[tx][ty].dir);//Dabun moca aizmugures x coord
					ty-=gety(MyWorld->Field[tx][ty].dir);//Dabun moca aizmugures y coord
					setValuesCell(&MyWorld->Field[tx][ty], EMPTY, -1, -1);//izdzes speletaja moca aizmugures datus
					moveTail(MyWorld, MyWorld->tails, CurPlayers[i].id, tx, ty, MyWorld->settings.playerCount, MyWorld->Field[tx][ty].dir);
					tx+=getx(MyWorld->Field[tx][ty].dir);//Dabun moca prieksas x coord
					ty+=gety(MyWorld->Field[tx][ty].dir);//Dabun moca prieksas y coord
					setValuesCell(&MyWorld->Field[tx][ty], BACK, dir, CurPlayers[i].id);//Pamaina veco prieksu uz aizmuguri
					*x = xd;
					*y = yd;
					setValuesCell(&MyWorld->Field[*x][*y], HEAD, dir, CurPlayers[i].id);//Pieliek jauno prieksu
					break;
				case HEAD://Ja mocis ietriecas moca galva tad abus mocus izdzes
					DeletePlayer(MyWorld, &CurPlayers[0], MyWorld->tails, i, MyWorld->settings.playerCount);
					DeletePlayerID(MyWorld, &CurPlayers[0], MyWorld->tails, MyWorld->settings.playerCount, MyWorld->Field[xd][yd].id);
					break;
				case BACK://Ja mocis ietriecas moca aizmugure tad abus mocus izdzes
					DeletePlayer(MyWorld, &CurPlayers[0], MyWorld->tails, i, MyWorld->settings.playerCount);
					DeletePlayerID(MyWorld, &CurPlayers[0], MyWorld->tails, MyWorld->settings.playerCount, MyWorld->Field[xd][yd].id);
					break;
				case TAIL://Ja mocis ietriecas aste tad astes gabalu un moci izdzes
					DeletePlayer(MyWorld, &CurPlayers[0], MyWorld->tails, i, MyWorld->settings.playerCount);
					HitTail(MyWorld, MyWorld->Players, MyWorld->tails, MyWorld->settings.playerCount, xd, yd);
					break;
				case BULLET://Ja lode ietriecas lode tad abas lodes izdzes
					DeletePlayer(MyWorld, &CurPlayers[0], MyWorld->tails, i, MyWorld->settings.playerCount);
					HitBullet(MyWorld, MyWorld->Bullets, MyWorld->bulletCountMax, xd, yd);
					break;
			}

		}
	}	
}
void clearField(World_t * world)
{
	int i = 0, k = 0;
	for(i; i < world->settings.width; i++)
	{
		for(k; k < world->settings.height; k++)
		{
			world->Field[i][k].type=EMPTY;
		}
	}
}
void calculateField(World_t * world)
{
	upd_player_t * player = getSelf(world);
	int x = player->x;
	int y = player->y;
	clearField(world);
//DEBUG("i: %d direction: %d",player->id , player->direction);
	MovePlayers(world);
	world->Field[x][y].type = HEAD;
}
#endif /* _WORLD_H_ */

