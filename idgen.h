
/* 
* 1: Init array with {-1, -1, -1 ,-1 ,-1 ... } 
*    when server is started.
* 2: Pass FD to gen_id, and it will put the FD in first
*    available player ID space.
* 3: Return the value of ID if OK.
*/

//Array of FDs, Index is ID
//Input values, need to be changed

/**
//Debug print

void print(){
    for(int i=0; i<PLAYER_COUNT; i++){
        printf ("Number: %d\n", tmp_id);
    }
**/

//Intialize array with, {-1,-1,-1,-1,-1...}
int* idInit(int player_count)
{
    int *list;
    int i;
    list = (int*) malloc((player_count)*sizeof(int));
    for(i=0; i<player_count; i++){
        list[i]=-1;
    }
    return list;
}

int idGen(int id, int player_count, int* list)
{
    int i;
    for(i=0; i<player_count; i++){
        if(list[i]==-1){
            list[i]=id;
			return i;
        }
    }
    return -1;
}
