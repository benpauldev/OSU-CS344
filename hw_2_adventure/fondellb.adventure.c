
/*
 *
 *    Title: Program 2 Adventure adventure.c
 *    Author: Benjamin fondell
 *    Date: 7/24/2017
 *    Description: Maze traversal game. Implements file handling and posix threading. 
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>


/*************************************************************
 GLOBAL VARiABLES
 *************************************************************/

#define MAX_ROOMS 7
#define MAX_CONNECTIONS 6

typedef enum {FALSE = 0, TRUE = 1} boolean;

enum roomType {START_ROOM,MID_ROOM,END_ROOM};

struct ROOM
{
    char name[40];
    int numOfConnections;
    struct ROOM* connections[MAX_CONNECTIONS];
    enum  roomType type;
};

struct ROOM rooms[MAX_ROOMS];
char   buffer[256];
pthread_mutex_t tMutex;
char *TFILE = "currentTime.txt";

/*************************************************************
    Function Prototypes
 *************************************************************/

void GetFolder();
void roomStructs();
void printRooms();
void Game();
void* MakeTimeFile();
boolean TimeThread();
void ReadTimeFile();

/*************************************************************
 *
 * Function: Main
 *
 *************************************************************/

int main()
{
    GetFolder();
    roomStructs();
    //printRooms();
    Game();
    return 0;
}

/*************************************************************
 *
 * Function: GetFolder
 *
 * Description: Finds the most recent directory made of room files.
 * stores the title of the directory with pid in the global buffer variable
 *
 *************************************************************/

void GetFolder()
{
    char *fileName = "fondellb.rooms.";             //Establishes name without pid
    char curDir[256];                               //Storage holder for current directory c string
    
    DIR *directory;                                 //directory pointer
    time_t mostRecent = 0;                          //variable to hold the most recent time
    
    struct dirent *dirBuffer = malloc(sizeof(struct dirent));       //holds directory information used for name
    struct stat *statbuffer = malloc(sizeof(struct stat));          //holds statistic used for time comparison
    
    memset(buffer,'\0',sizeof(buffer));            //prevents overlapping data on multiple calls
    memset(curDir, '\0', sizeof(curDir));
    
    getcwd(curDir,sizeof(curDir));                 //stores pathname in curDir
    directory = opendir(curDir);                   //opens directory at path
    
    if(directory == NULL)
    {
        printf("Error: Cannot find directory.");
        return;
    }
    else
        while ((dirBuffer = readdir(directory)))                    //loops through directories looking for the most recent modified
        {
            if (strstr(dirBuffer->d_name,fileName) != NULL)
            {
                
                stat(dirBuffer->d_name, statbuffer);
            
                if (statbuffer->st_mtime > mostRecent)              //time comparison
                {
                    mostRecent = statbuffer->st_mtime;
                    strcpy(buffer, dirBuffer->d_name);              //sets name of most recent directory into global buffer
                }
               
            }
        }
    
    //printf("This is the most recent directory: %s\n",buffer);

}

/*************************************************************
 *
 * Function: roomStructs()
 *
 * Description: used for making an array of stucts from the file similar to how
 * the rooms are made into files in buildrooms.c
 *
 *************************************************************/

void roomStructs()
{
    DIR *directory;                     //directory pointer
    struct dirent *entity;              //entity holds dirent information
    int counter = 0;
    char line[256];                     //line buffer
    char value[256];                    //value buffer these are used for line/value pairs
    FILE *roomFile;                     //file pointer to room files in directory
  
    
   // memset(buffer,'\0',sizeof(buffer));
   // strcpy(buffer, "fondellb.rooms.10656");
    
    //printf("Buffer holds: %s\n",buffer);
    
    for(int i = 0;i < MAX_ROOMS;i++)                        //initializes an array of structs
    {
        memset(rooms[i].name,'\0',sizeof(rooms[i].name));
        rooms[i].numOfConnections = 0;
      
        for(int j = 0; j < MAX_CONNECTIONS; j++)
        {
            rooms[i].connections[j] = NULL;
        }
    }
    
    if((directory = opendir(buffer)) != NULL)                   // fills the array with fine names (room names)
    {
        
        while ((entity = readdir(directory)) != NULL)
        {
            
            //printf("Entity Name: %s\n",entity->d_name);
            
            if ((strlen(entity->d_name) > 2))
            {
                strcpy(rooms[counter].name, entity->d_name);
                //printf("This room: %s\n",rooms[counter].name);
                counter++;
            }
        }
    }
    
    chdir(buffer);        //changes the current directory to the directory stored in buffer

    for (int i = 0; i < MAX_ROOMS; i++)                     //loops through all files in directory
    {
        //printf("Trying to open: %s\n",rooms[i].name);
        
        errno = 0;
        
        roomFile = fopen(rooms[i].name,"r");
        
        //if(roomFile == NULL)
        //{
            //printf("%s file did not open\n",rooms[i].name);
            //printf("Error %d \n",errno);
            
        //}
        //else
            //printf("Working\n");
        
        memset(line,'\0',sizeof(line));                                 //resets buffers to prevent garbage
        memset(value,'\0',sizeof(value));
    
        while((fgets(line, sizeof(line),roomFile) != NULL))             //loops through lines of all files filling struct array with values stored in files
        {
            strtok(line,":");                                           //used to break the lines up on the colon
            strcpy(value, strtok(NULL,""));                             //left side is value
            value[strlen(value) - 1] = '\0';                            //prevents off by one and adds endline
            line[strlen(line)] = '\0';                                  //adds end line
                      
            //printf("label: %s\n",line);
            //printf("value: %s\n",value);
            
            if(strcmp(line,"Type") == 0)                                //sets types
            {
                
                if(strcmp(value," Start Room") == 0)
                {
                    rooms[i].type = START_ROOM;
                }
                else if (strcmp(value," Finish Room") == 0)
                {
                    rooms[i].type = END_ROOM;

                }
                else
                {
                    rooms[i].type = MID_ROOM;

                }
            
            }
            else if (strncmp(line, "Connection" ,10) == 0)              //sets connections, looks at only "connection" not the index of connection
            {
                char* value_chopped = value + 1;                        //chops the leading whitespace of values
                
                for (int j = 0; j<MAX_ROOMS ; j++)
                {
                    if(strcmp(value_chopped,rooms[j].name) == 0)
                    {
                        //printf("Connection Added");
                        
                        int totalConnections = rooms[i].numOfConnections;
                        
                        rooms[i].connections[totalConnections] = &rooms[j];
                        rooms[i].numOfConnections++;
                    }
                }
                
            }
        
      }
    fclose(roomFile);
    }

chdir("..");

}

/*************************************************************
 *
 * Function: printRooms
 *
 * Description: prints the array of struct Rooms used for debugging
 *
 *************************************************************/

void printRooms(){
    
    int i;
    int j;
    int count;
    
    for(i=0;i<MAX_ROOMS;i++)                        //loops through the array printing the rooms and printing off the connections and types at each
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
 * Function: getPosition
 *
 * Description: Helper function for finding the position in the graph of rooms
 *
 *************************************************************/

int getPosition(char input[])
{
    
    for(int i = 0; i < MAX_ROOMS; i++)          //loops through the room returning the index of the struct w/ matching name
    {
        if (strcmp(rooms[i].name,input)==0)
        {
            return i;
        }
    }
    
    return -1;
    
}

/*************************************************************
 *
 * Function: Game()
 *
 * Description: This is where the game is run and driven. This function implements multithreading to
 * allow breaks to call time functions.
 *
 *************************************************************/

void Game()
{
    int traversal[512];             //array holds sequence of steps
    struct ROOM curRoom;            //current room struct
    int currentPos = 0;
    int step = 0;
    boolean Playing = TRUE;         //for passing through while loop and ending the while loop
    boolean found = FALSE;          //used to trigger an if statement
    char *input[128];               //buffer holding getline input
    
        for(int i =0;i< MAX_ROOMS; i++)         //finds start room
        {
            if(rooms[i].type == START_ROOM)
            {
                traversal[step] = i;
            }
        }
    
    //printf("The starting room is: %s \n",rooms[traversal[currentPos]].name);
    
    while (Playing)                                 //loop to display gameplay and provide prompts
    {
        memset(input, '\0', sizeof(input));
        
        currentPos = traversal[step];               //current = startroom if first loop otherwise last item in array
        curRoom = rooms[currentPos];                //sets current room to the room ar cuurentPos
        
        printf("CURRENT LOCATION: %s\n",curRoom.name);
        
        printf("POSSIBLE CONNECTIONS: ");
        
        int i;
        
        for( i = 0; i < curRoom.numOfConnections - 1; i++)      //prints connections
        {
            printf("%s, ",curRoom.connections[i]->name);
        }
        
        printf("%s.\n",curRoom.connections[i]->name);
        
        printf("Where to? >");                                  //promps player input
        
        size_t insize = 100;                                    //size of getline max
        
        getline(input, &insize, stdin);                         //gets user input from stdin
        
        strtok(*input, "\n");                                   //cuts off newline
        
        printf("\n\n");
        
        for(i = 0; i < curRoom.numOfConnections; i++)           //loops through connections comparing to user input
        {
            //printf("%s",curRoom.connections[i]->name);
            
            if(strcmp(*input,curRoom.connections[i]->name) == 0)    //if the same a match was found
            {
                found = TRUE;                                       //updates all variable and sets found to true
                ++step;
                traversal[step] = getPosition(*input);
                currentPos = traversal[step];
                curRoom = rooms[currentPos];
                
                if(curRoom.type == END_ROOM)                            //if the matched room is the end room display end messages
                {
                    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",step + 1);
                    for (int j = 0; j < step + 1; j++)                                          //print sequence of moves
                    {
                        printf("%s\n",rooms[traversal[j]].name);
                    }
                    printf("\n\n");
                    
                    Playing = FALSE;
                    return;
                }
            }
        }
        if (strcmp(*input, "time") == 0)                                    //if time was entered thread for time
        {
            if (TimeThread() == TRUE)
            {
                ReadTimeFile();                                             //call to read time file
            }
        }
        else if (found == FALSE)                                            //if no match display error and prompt input again
        {
            printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        }
        
    }
}

/*************************************************************
 *
 * Function: ReadTimeFile
 *
 * Description: reads time from the currenttime.txt file where the current time is placed by another function
 *
 *************************************************************/

void ReadTimeFile()
{
    char timeBuffer[128];                           //buffer for holding time
    memset(timeBuffer,'\0',sizeof(timeBuffer));
    
    FILE *time_file;
    
    errno = 0;
    
    time_file= fopen(TFILE,"r");            //opens global time file
    
    if(time_file == NULL)                       //error if file not opened
    {
        printf("%s could not open.\n",TFILE);
        printf("Error %d \n",errno);
        return;
    }
    
    
    while(fgets(timeBuffer,128,time_file) != NULL)          //prints time from file
    {
        printf("%s\n",timeBuffer);
    }
    fclose(time_file);          //cleans up opened file
}

/*************************************************************
 *
 * Function: maketimefile()
 *
 * Description: writes the time to currenttime.txt file
 *
 *
 *
 *************************************************************/

void* MakeTimeFile()
{
    char timeBuffer[128];
    memset(timeBuffer,'\0',sizeof(timeBuffer));
    
    time_t curtime;
    
    struct tm * tdata;
    FILE *time_file;
    
 
    time(&curtime);
    
    tdata= localtime(&curtime);
    
    strftime(timeBuffer,256, "%I:%M%P %A, %B %d, %Y", tdata);       //parses time data into readible form
                                                                    //ref: http://search.cpan.org/dist/TimeDate/lib/Date/Format.pm
    
    time_file = fopen(TFILE,"w");
    
    errno = 0;
    
    if(TFILE == NULL)                   //handles error if file could not open
    {
        printf("%s could not open.\n",TFILE);
        printf("Error %d \n",errno);
        
    }
    
    fprintf(time_file,"%s\n",timeBuffer);
    
    fclose(time_file);
    
    return NULL;
}

/*************************************************************
 *
 * Function:timethread()
 *
 * Description: function for handling the time thread implementation
 * sourced significantly from discussion boards piazza and stack overflow.
 *
 *************************************************************/

boolean TimeThread()
{
    //printf("Time Thread Entered\n");
    
    pthread_t writethread;      //creats posix thread
    
    pthread_mutex_lock(&tMutex);       //locks mutex
    
    errno = 0;
    
    if(pthread_create(&writethread,NULL,MakeTimeFile,NULL) != 0)  //new thread in call process, no attr structures, MakeTimeFile is passed as start routine
    {
        printf("Thread Error.");
        printf("Error %d \n",errno);
        return FALSE;
    }
    
   
    pthread_mutex_unlock(&tMutex);  //unlocks mutex
   
    pthread_join(writethread,NULL); //waits for thread to terminate
    
    return TRUE;  //returns to game which prints the time 
}

















