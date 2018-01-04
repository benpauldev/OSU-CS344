/*
 *
 *    Title: Program 2 Adventure Buildrooms.c
 *    Author: Benjamin fondell
 *    Date: 7/24/2017
 *    Description: Builds a maze of room in the form of a connected graph of files. Files act as rooms and will be used for a maze traversal game.
 *
 *
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

/*************************************************************
 GLOBAL VARiABLES
 *************************************************************/

#define MAX_ROOMS 7
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define SIZE_ROOM_LIST 10

typedef enum {FALSE = 0, TRUE = 1} boolean; // found useful as no T/F in raw c

char *roomNames[10] = {"Ballroom","Dungeon","Boiler Room","Study","Tower Room","Morgue","Creepy Kitchen","Dining Room","Sleeping Chamber","Laboratory"};

enum roomType {START_ROOM,MID_ROOM,END_ROOM};

struct ROOM
{
    char name[40];
    int numOfConnections;
    struct ROOM* connections[MAX_CONNECTIONS];
    enum  roomType type;
};

struct ROOM rooms[MAX_ROOMS];
int   initialRooms[10];
char    buffer[256];

/*************************************************************
 Function Prototypes
 *************************************************************/

void initRooms();
void makeRooms();
int GetRandomRoom();
int IsGraphFull();
void AddRandomConnection();
int IsSameRoom(int, int);
int CanAddConnectionFrom(int);
void ConnectRoom(int,int);
void printRooms();
int GetRandomRoomFromRooms();
int IsAlreadyConnected(int, int);
void initRoomConnections(int);
int IsConnected(int,int);
void makeRoomDir();
void makeRoomFiles();

/*************************************************************
 *
 *  MAIN
 *
 *************************************************************/

int main()
{
    srand((unsigned int)(time(NULL)));
    initRooms();
    makeRooms();
    makeRoomFiles();
    return 0;
}

/*************************************************************
 *
 * Function: initRooms
 *
 * Description: Initializes rooms, fills with random names, types, and initializes
 * array of connections to 0.
 *
 *************************************************************/

void initRooms()
{
    int i;
    
    for(i = 0; i < MAX_ROOMS;i++)
    {
        initialRooms[i] = 0;
    }
    
        for(i = 0; i < MAX_ROOMS;i++)
        {
            rooms[i].numOfConnections = 0;
        
            initRoomConnections(i);    //initializes connections of each room.
        
        while(TRUE){
            
            int randroom = GetRandomRoom();     //gets random room from list
            
            if(initialRooms[randroom] == FALSE)
            {
                initialRooms[randroom] = TRUE;
                
                strcpy(rooms[i].name,roomNames[randroom]);      //names room from room names
                
                rooms[i].type = MID_ROOM;                       //sets as default to mid_room
                
                break;
            }
        }
    }
   
    rooms[0].type = START_ROOM;                                 //defines start room as first in list
    
    rooms[MAX_ROOMS - 1].type = END_ROOM;                       //defines end room as the room at end of array
}

/*************************************************************
 *
 * Function: initRoomConnections
 *
 * Description: This was originally a nested for loop but was
 *   added as a helper function to prevent seg fault.
 *   initializes connections of each room to 0.
 *
 *************************************************************/

void initRoomConnections(int x)
{
    int i;
    for(i = 0; i < MAX_CONNECTIONS; i++)
    {
        rooms[x].connections[i] = NULL;
    }
}

/*************************************************************
 *
 * Function: makeRooms()
 *
 * Description: This function is used to make the graph of room connections
 * could be packaged into another function but was recommended by class
 *
 *************************************************************/

void makeRooms()
{
    while (IsGraphFull() == 0)              //when grah has satisfied the number of connections at each room within the minimum and maximum
                                            //it completes
    {
        //printf("Adding connection.\n");
        AddRandomConnection();
    }
}

/*************************************************************
 *
 * Function: getRandomRoom()
 *
 * Description: Helper function for getting and returning a random room from the array
 * of rooms. this is the function to get a random name before names are given.
 *
 *************************************************************/

int GetRandomRoom()
{
    int position;
    
    do{
        position = rand() % SIZE_ROOM_LIST;         //uses random library function to return int
        
    }while(CanAddConnectionFrom(position) == 0);
    
    return position;
}

/*************************************************************
 *
 * Function: GetRandomRoomFromRooms()
 *
 * Description:Helper function for getting and returning a random room from the array
 * of rooms. Function is used to pull random names from list of rooms after names are given.
 * list is size of 7.
 *
 *************************************************************/

int GetRandomRoomFromRooms()
{
    int position;
    
    do{
        position = rand() % MAX_ROOMS;
        
    }while(CanAddConnectionFrom(position) == 0);
    
    return position;
}


/*************************************************************
 *
 * Function: IsGraphFull()
 *
 * Description: Helper function for determining if graph is full.
 * Checks that each room has connections in the range of 3 and 6 to other rooms.
 *
 *
 *************************************************************/

int IsGraphFull()
{
    //printf("Checking graph is not full.\n");
    
    
    int allFull = 0;   //allfull counts rooms that satisfy the conditions
    
    for(int i=0;i<MAX_ROOMS;i++){
        
            //printf("%i\n",allFull);
        
            if(rooms[i].numOfConnections <= MAX_CONNECTIONS && rooms[i].numOfConnections >= MIN_CONNECTIONS)
                allFull++;
            else
                return 0;
    }
    
    if (allFull == MAX_ROOMS)
    {
        return 1;
    }
    
    return 0;
}

/*************************************************************
 *
 * Function: AddRandomConnection()
 *
 * Description: adds a random connection between two rooms
 *checks that the room it adds connection to is not the same room and is not already connected.
 *
 *************************************************************/

void AddRandomConnection()
{
    int A = 1;
    int B;
        
    while(A)
    {
        A = GetRandomRoomFromRooms();
        //printf("Looking at room %i\n",A);
        if (CanAddConnectionFrom(A) == 1)
            break;
    }
    
    do
    {
        B = GetRandomRoomFromRooms();
         //printf(" Now Looking at room %i\n",B);
    }
    while(CanAddConnectionFrom(B) == 0 || IsSameRoom(A,B) == 1);
    
    ConnectRoom(A, B);

    ConnectRoom(B, A);
}

/*************************************************************
 *
 * Function: CanAddConnectionFrom()
 *
 * Description:Helper function for error checking maximum connections.
 *recommended by the class however not that useful.
 *
 *************************************************************/

int CanAddConnectionFrom(int x)
{
    //printf("Checking if we can add connection for %i.\n",x);
    
    if(rooms[x].numOfConnections >= MAX_CONNECTIONS)
        return 0;
    
    
    
    //printf("We can add connection for %i!\n",x);
    return 1;
    
    
}

/*************************************************************
 *
 * Function: IsAlreadyConnected(0
 *
 * Description: Compares that two rooms are not already connected. 
 *
 *
 *************************************************************/

int IsAlreadyConnected(int A, int B)
{
    int i;
    
   
    if(rooms[A].numOfConnections == MAX_CONNECTIONS)
    {
        return 1;
    }
    
  
    for(i = 0; i < rooms[A].numOfConnections;i++){
   
        if(rooms[A].connections[i] == NULL)
        {
            return 0;
        }
        else if(strcmp(rooms[A].connections[i]->name,rooms[B].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*************************************************************
 *
 * Function: IsConnected()
 *
 * Description: Helper function to see if rooms are already connected to eachother to avoid repeat connections
 *
 *
 *************************************************************/

int IsConnected(int A, int B)
{
    if(IsAlreadyConnected(A,B) == 1)
    {
        
        return 1;
    }
    else if(strcmp(rooms[A].name, rooms[B].name)== 0)
    {
        
        return 1;
    }
    else{
        return 0;
    }
}

/*************************************************************
 *
 * Function: ConnectRoom()
 *
 * Description: Conncts two rooms. loops until finds a place where there is an open connection and then
 * adds connection at that position.
 *
 *
 *************************************************************/
void ConnectRoom(int A,int B)
{
    
    int connected = 0;
   /* int test1,
         test2;
    
    test1 = IsAlreadyConnected(A,B);
    test2 = IsSameRoom(A,B);*/
    
    //printf("test already connected: %i\n test same room: %i",test1,test2);
    do
    {
        //printf("looping");
        if(IsConnected(A,B) == 0)
        {
            
            int connect1 = rooms[A].numOfConnections;       //holds value at room number of connections.
            int connnect2 = rooms[B].numOfConnections;
            
            rooms[A].connections[connect1] = &rooms[B];     //connect room to address of other room, adds to end of connections array
            rooms[B].connections[connnect2] = &rooms[A];
            
            rooms[A].numOfConnections++;                    //increment connectons
            rooms[B].numOfConnections++;
            
            connected = TRUE;
        }
        else
            break;
    }
    while(connected == FALSE);
    
}

/*************************************************************
 *
 * Function: ISSameROOM
 *
 * Description: Helper function to check if two rooms are the same.
 *
 *************************************************************/

int IsSameRoom(int A, int B)
{
    if(A==B)
        return 1;
    else
        return 0;
}

/*************************************************************
 *
 * Function: printRooms()
 *
 * Description: Prints the array of struct Rooms, used for debugging.
 *
 *
 *************************************************************/

void printRooms(){
    
    int i;
    int j;
    int count;
    
    for(i=0;i<MAX_ROOMS;i++)
    {
        count = 1;
        
        printf("ROOM NAME: %s\n",rooms[i].name);
    
        
        for (j = 0; j<MAX_CONNECTIONS; j++)
        {
            if(rooms[i].connections[j] != NULL)
                printf("Connection %i: %s\n", count,rooms[i].connections[j]->name);
                count++;
        }
        switch (rooms[i].type)
        {
            case 0:
                printf("Type: Start Room\n");
                break;
                
            case 2:
                printf("Type: Finish Room\n");
                break;
                
            default:
                printf("Type: Mid Room\n");
                break;
        }
        printf("\n\n");
    }
    
}

/*************************************************************
 *
 * Function:makeRoomFiles()
 *
 * Description: makes the files of each room struct according o format
 *
 *
 *************************************************************/


void makeRoomFiles()
{
    FILE *roomFile;
    int i,j,count;
    char folderbuffer[256];
    
    sprintf(folderbuffer,"./fondellb.rooms.%d",getpid());
    
    makeRoomDir();
    
    if(chdir(folderbuffer) != 0){
        
        return;
    }
    
    for(i=0;i<MAX_ROOMS;i++)
    {
        count = 1;
        
        roomFile = fopen(rooms[i].name,"w");
        
        fprintf(roomFile,"ROOM NAME: %s\n",rooms[i].name);
        
        
        for (j = 0; j<MAX_CONNECTIONS; j++)
        {
            if(rooms[i].connections[j] != NULL)
                fprintf(roomFile,"Connection %i: %s\n", count,rooms[i].connections[j]->name);
                count++;
        }
        switch (rooms[i].type)
        {
            case 0:
                fprintf(roomFile,"Type: Start Room\n");
                break;
                
            case 2:
                fprintf(roomFile,"Type: Finish Room\n");
                break;
                
            case 1:
                fprintf(roomFile,"Type: Mid Room\n");
                break;
                
            default:
                 fprintf(roomFile,"Type: NULL\n");
        }
        fclose(roomFile);
    }
 
}


/*************************************************************
 *
 * Function: makeRoomDir
 *
 * Description: Creates a directory for the room files and sets permissions.
 *  also labels directory name with unique pid.
 *
 *************************************************************/

void makeRoomDir()
{
    char* staticDirName = "fondellb.rooms."; //will add pid
    
    int pid = getpid();
    
    int openPermissions = 0777;   //open permissions for r,w,x for user group and other
    
    sprintf(buffer,"%s%d",staticDirName,pid); //builds directory name with pid
    
    //printf("%s",buffer);
    
    mkdir(buffer,openPermissions); // creates folder with open permissions
}










