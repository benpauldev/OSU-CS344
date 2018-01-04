#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include "otp_header.h"


void error(const char *msg);
void encrypt_function(int sockfd);






int main(int argc, char *argv[])
{
   
    int sockfd,                                     //initial socket fd
        next_sockfd,                                //next socket fd to deal with
        portno,                                     //port number
        pid;                                        // process id
    
    socklen_t client_length;
    struct sockaddr_in serv_addr, client_address;
    
  
    if (argc < 2)                                               //handles the case that a port was not given
    {
        fprintf(stderr,"Error no port was specified.\n");
        exit(1);
    }
    
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                   //creates a new socket and file descriptor
    
    if (sockfd < 0)                                             //if sockfd is null then no socket was opened
    {
        error("Error opening socket.");
    }
    
    memset((char *) &serv_addr,'\0', sizeof(serv_addr));                            //clear server address memory
    
     portno = atoi(argv[1]);
    
    serv_addr.sin_family = AF_INET;                                                 //initialize server address
    serv_addr.sin_addr.s_addr = INADDR_ANY;                                         //allows acceptance of connections from all ip's
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)        //bind socket to port
    {
        error("Error binding socket.");
    }
    
    
    if (listen(sockfd,5) < 0)                                                       //listen for up to 5 connections
    {
        error("Error: hull breach at listen.");
    }
    
    client_length = sizeof(client_address);                                                      //size of address that we will connect
    
    while (1) {
        
        next_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_length);      //accept a connection blocking if one is not available
                                                                                                // until a connection is made
        
        
        if (next_sockfd < 0)                                                  //error checking accept fd
        {
            error("Error: hull breach at accept.");
        }
        
        pid = fork();                                                         //call fork process
        
        if (pid < 0)
        {
            error("Error: unable to fork.");
            
        }
        if (pid == 0)                                                          //child process
        {
            close(sockfd);                                                     //close socket connection
            fcntl(next_sockfd, F_SETFL, O_NONBLOCK);                           //set sockt to non-blocking
            encrypt_function(next_sockfd);                                           //call encrypt on socket connection
            exit(0);                                                           //exit
        }
        else                                                                   //if not a child process close the socket
        {
            close(next_sockfd);
        }
    }
    
    close(sockfd);
    return 0; 
}
