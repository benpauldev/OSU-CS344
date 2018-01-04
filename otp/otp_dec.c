#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "otp_header.h"

void error(const char *msg);
void getText(char *input, char* filename);


int main(int argc, char *argv[])
{
    
    ssize_t  portno;                        //port number
    ssize_t test;                       //test
    ssize_t sockfd;                         //socket file descriptor
    struct sockaddr_in serv_addr;       //server address struct
    struct hostent *server;             //host struct
    
    
    char char_buffer[256];                   //charachter buffer for rec from socket
    
    if (argc < 4)
    {
        fprintf(stderr,"usage: %s requires: text file, key, port number\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    portno = atoi(argv[3]);                                 //collect port number of encription from arguments list
    
 
    char *key_buffer = malloc(sizeof(char*) * 70000);          //create a buffer for key text
    
    getTextDecode(key_buffer, argv[2]);                             //get text from key hold in key_buffer
   
    char *file_buffer = malloc(sizeof(char*) * 70000);         //create a buffer for file text
    
    getTextDecode(file_buffer, argv[1]);                            //get text from filehold in file_buffer
    
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);               //create socket
    
    if (sockfd < 0)                                         //error if failed to create socket
    {
       
        error("error creating socket");
    }
    
    server = gethostbyname("localhost");                                                    //target localhost server
    if (server == NULL)                                                                     //error if for some reason cant find localhost
    {
        error("Error finding hostname.");
        
    }
    
    
    memset((char *) &serv_addr,'\0', sizeof(serv_addr));                                    //clear memory that held server address struct
    
    serv_addr.sin_family = AF_INET;                                                         //sets server address family
    serv_addr.sin_port = htons(portno);                                                     //set port address

    
    memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);    //copy server address of localhost to host struct
   
    
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)               //error if connecting server returns -1
    {
        
        error("Error connecting socket.");
    }
    


    
    test = write(sockfd,key_buffer,strlen(key_buffer));                 //send key to socket
    
    if (test < 0 || test < strlen(key_buffer))                                                  //error if send key failed
    {
        error("Error sending to socket.");
    }
    
    sleep(1);                                                     //pause for race conditions
    
    test = write(sockfd,file_buffer,strlen(file_buffer));              //send file to socket
   
    if(test < 0)                                                  //error if send file failed
    {
        error("ERROR writing to socket");
    }
    
    memset(char_buffer, '\0', 256);                                  //clean buffer
    
    while((test = read(sockfd,char_buffer,255)) > 0)               //while reciving from socket
    {
        if (test < 0)
        {
            error("ERROR reading from socket");                 //error recieving from socket
        }
        
        printf("%s", char_buffer);
        memset(char_buffer,'\0',256);                                //clean buffer
    }
    
    printf("\n");                                               //advance cli
    close(sockfd);                                             //close socket
    
    return 0;
}
