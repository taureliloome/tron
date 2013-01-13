#include <stdio>
#define PLAYER_COUNT 8

/* 
* 1: Init array with {-1, -1, -1 ,-1 ,-1 ... } 
*    when server is started.
* 2: Pass FD to gen_id, and it will put the FD in first
*    available player ID space.
* 3: Return the value of ID if OK.
*/

//Array of FDs, Index is ID
list = (int*) malloc((PLAYER_COUNT)*sizeof(int));
//Input values, need to be changed
tmp_fd=1;
tmp_id=0;

//Debug print
void print(){
    for(int i=0; i<PLAYER_COUNT; i++){
        printf ("Number: %d\n", tmp_id);
    }

//Intialize array with, {-1,-1,-1,-1,-1...}
void init(){
    for(int i=0; i<PLAYER_COUNT; i++){
        list[i]=-1;
    }

void gen_id(){
    for(int i=0; i<PLAYER_COUNT; i++){
        if(list[i]==-1){
            list[i]=tmp_fd;
        }
    }
    return i;
}

// Return the FD of given ID
int get_fd(){
    return list[tmp_id];
}
