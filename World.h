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

tail_t * createtail(World_t *someWorld, int id)
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
	    	someWorld->Players[pc].gameover = 1;         // 1 - miris, 0 - spēlē

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


	/* Handling player list */

	playerHeader = (upd_player_header_t *) iterator;
	iterator += sizeof(upd_player_header_t);

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

	/* Handling Bullet list */

	bulletHeader = (upd_bullet_header_t *) iterator;
	iterator += sizeof(upd_bullet_header_t);
	
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

	/* Handling tails */

	tailHeader = (upd_total_tail_header_t*) iterator;
	iterator += sizeof(upd_total_tail_header_t);

	for(i = 0; i < tailHeader->totalTailLength; i++){
		temptail = (upd_tail_header_t*) iterator;
		existtail = findtail(someWorld , temptail->id);	

		/*TODO: VEIDOT ASTES UZ SPELETAJA VEIDOSANU	 kkur tur augsa*/

		if(existtail != NULL){  //visas astes ir izveidotas kad tiek veidots speletajs 
			
			existtail->length = temptail->tailCount;
			
			for( k = 0; k < temptail->tailCount; k++){
				tempCell = (upd_tail_t *) iterator;
				existtail->cells[k].x = tempCell->x;
				existtail->cells[k].y =	tempCell->y;
				tempCell += sizeof(upd_tail_t);
			}
		}
		
		iterator += sizeof(upd_tail_header_t);
	}

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
		
	void *packet,*tailPacket;

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
	if ( alloc_size >= sizeof(upd_tail_t) * cellLengthSumm + sizeof(upd_tail_header_t) * someWorld->tailCountAlive )
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
		memcpy( packet , &totaltailHeader, sizeof(&totaltailHeader) );
		packet += sizeof(&totaltailHeader);
	}

	if ( playerList ) {
		memcpy(packet, tailPacket, sizeof(tailPacket) );
		size += sizeof( tailPacket );
	}

	*length = sizeof( packet);
	return packet;
}



void CreateClientWorld(World_t *someWorld,conn_resp_t * Params)
{
	int k,i;
	WorldCell_t tempCell;
	someWorld->settings.height=Params->height;
	someWorld->settings.width=Params->width;
// WARNING SHIT CODED 2d ARRAY REFACTOR NAHOOOJ
	//someWorld->Field = malloc(Params->width * sizeof(struct WorldCell));

    //for (k = 0; k < Params->height; k++) 
    //{
	//	someWorld->Field[k] = malloc(sizeof(struct WorldCell*));
	//	memset(someWorld->Field[k], 0, sizeof(struct WorldCell*));
   // }

	someWorld->Players = (upd_player_t*)malloc(someWorld->settings.playerCount * sizeof(upd_player_t));

	createPlayer(someWorld , Params->id,40,10); //izveidojam pirmo speletaju, to kuru vadis tekošais klients

	createtail(someWorld, Params->id); //izveidojam vinam asti garuma 0



//	someWorld->Field = malloc(Params->width * Params->height * sizeof(void**));
//
//   	for (k = 0; k < Params->width; k++) 
//  	{
//		someWorld->Field[k] = malloc(Params->height * sizeof(void*));
//		for(i = 0; i < Params->height; i++)
//		{
//			tempCell = someWorld->Field[k][i];
//			tempCell.x = k;
//			tempCell.y = i;
//		}
//		
//    	}
//
//	someWorld->Players = (struct UpdatePlayer*)malloc(someWorld->playerCountMax * sizeof(struct UpdatePlayer));


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

bulletHitTail(World_t *MyWorld, upd_player_t *Players, tail_t *Tails, int count, int x, int y)
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

bulletHitBullet(World_t *MyWorld, upd_bullet_t *Bullets, int count, int x, int y)
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
						bulletHitTail(MyWorld, MyWorld->Players, MyWorld->tails, MyWorld->settings.playerCount, xd, yd);
						*x = -1;
						*y = -1;
						break;
					case BULLET://Ja lode ietriecas lode tad abas lodes izdzes
						setValuesCell(&MyWorld->Field[*x][*y], EMPTY, -1, -1);
						bulletHitBullet(MyWorld, Bullets, MyWorld->bulletCountMax, xd, yd);
						*x = -1;
						*y = -1;
						break;
				}
				
			}
/*
				if (MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
					[Bullets[i].y+gety(Bullets[i].direction)].type != EMPTY)
				{
					if (MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
						[Bullets[i].y+gety(Bullets[i].direction)].type == BULLET)
					{
						
					}
					else
					if (MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
						[Bullets[i].y+gety(Bullets[i].direction)].type == tail)
					{
	
					}
					else
					if (MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
						[Bullets[i].y+gety(Bullets[i].direction)].type == HEAD)
					{
					}

				}
				else
				if ((MyWorld->Field[Bullets[i].x+getx(Bullets[i].direction)]
					[Bullets[i].y+gety(Bullets[i].direction)]).type == HEAD)
				{

				}
*/
			
	}	
}

void DeletePlayer(World_t *MyWorld, int ID)
{
	int i;
	upd_player_t *DelPlayer;
	DelPlayer = getSelf(MyWorld);
	for (i=0;i<MyWorld->settings.playerCount;i++)
	{
		if (DelPlayer[i].id == ID)
		{
			DelPlayer[i].gameover=1;

			//free(MyWorld->Field[DelPlayer[i].x][DelPlayer[i].y]);
			//MyWorld->Field[DelPlayer[i].x][DelPlayer[i].y] = NULL;

			MyWorld->Field[DelPlayer[i].x][DelPlayer[i].y].type=EMPTY;

			break;
		}
	}
}

void MovePlayers(World_t *MyWorld)
{
	upd_player_t *CurPlayers;
	CurPlayers = getSelf(MyWorld);
	int i;
	for (i=0;i<MyWorld->settings.playerCount;i++)
	{
		if (CurPlayers[i].gameover == 0)
		{

			if (MyWorld->Field[CurPlayers[i].x+getx(CurPlayers[i].direction)][CurPlayers[i].y+gety(CurPlayers[i].direction)].type == EMPTY)
			{
				CurPlayers[i].x+=getx(CurPlayers[i].direction);
				CurPlayers[i].y+=gety(CurPlayers[i].direction);
			}
			else
			{
				DeletePlayer(MyWorld, CurPlayers[i].id);
			}

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
	MyWorld->bulletCooldown=20;
	MyWorld->bulletCountAlive=0;
	MyWorld->timeout=100;
	MyWorld->playerCount=4;
	MyWorld->playerCountAlive=0;
	bulletMultiplier = maxSide/(MyWorld->bulletCooldown*MyWorld->bulletSpeed);
	if (maxSide%(MyWorld->bulletCooldown*MyWorld->bulletSpeed)>0)
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
	MyWorld->bulletCooldown=Params->bulletCooldown;
	MyWorld->timeout=Params->timeout;	
	MyWorld->playerCount=Params->playerCount;
	bulletMultiplier = maxSide/(MyWorld->bulletCooldown*MyWorld->bulletSpeed);
	if (maxSide%(MyWorld->bulletCooldown*MyWorld->bulletSpeed)>0)
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

