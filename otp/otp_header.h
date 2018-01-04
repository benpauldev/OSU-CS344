
#ifndef otp_header_h
#define otp_header_h

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

void error(const char *msg);
void decrypt_function(int sockfd);
void encrypt_function(int sockfd);
void getTextDecode(char *input, char* filename);
void getTextEncode(char *input, char* filename);


void error(const char *msg)
{
    printf("%s", msg);
    exit(EXIT_FAILURE);
}




void getTextEncode(char *input, char *filename) {
    
    
    FILE *file = fopen(filename, "r");                                 //file read pointer
    
    if(file == NULL)                                                    //if file cannot be opened
    {
        error("File: open error");
    }
    
    char character;
    
    int index = 0;
    
    while((character = fgetc(file)) != EOF)                            //loop the file collecting the charachters and storing in input
    {
        
        if(((character - 'A') >= 0 && (character - 'A') <= 25))       //take any charachter that is in a,b,c...z
        {
            input[index] = character;
        }
        else if((character == ' ') || (character == '\n'))            //also take soaces and endline charachters
        {
            input[index] = character;
        }
        else                                                          //if a charchter was not a ' ', '\n', or a,b,c... error reading char
        {
            error("File: character read error.");
        }
        
        index++;
    }
    
    input[index] = '\n';                                             //end file with \n
    input[index + 1] = '\0';                                        //end file with null
    
    fclose(file);
}

void getTextDecode(char *input, char *filename) {
    
    
    FILE *file = fopen(filename, "r");                                 //file read pointer
    
    if(file == NULL)                                                    //if file cannot be opened
    {
        error("File: open error");
    }
    
    char character;
    
    int index = 0;
    
    while((character = fgetc(file)) != EOF)                            //loop the file collecting the charachters and storing in input
    {
        
        if(((character - 'A') >= 0 && (character - 'A') <= 25))       //take any charachter that is in a,b,c...z
        {
            input[index] = character;
        }
        else if((character == ' ') || (character == '\n'))            //also take soaces and endline charachters
        {
            input[index] = character;
        }
        else                                                          //if a charchter was not a ' ', '\n', or a,b,c... error reading char
        {
            error("File: character read error.");
        }
        
        index++;
    }
    
    input[index] = '\n';                                             //end file with \n
    input[index + 1] = '\0';                                        //end file with null
    
    fclose(file);
}

void encrypt_function(int sockfd) {
    
    //printf("ive reached encrypt!!!!!!!!!!!!!!");
    
    
    ssize_t receiveno,                                          //receive fd
    sendno;                                             //send fd
    int  key_size = 0;                                          //size of key
    int text_size = 0;                                          //size of text to encrypt
    bool not_EndLine = true;                                      //this is a bool for checking endline
    char input_buffer[1000];
    int* key_buffer = malloc(sizeof(int) * 70000);                //holds key
    int* text_buffer = malloc(sizeof(int) * 70000);              //holds plain text
    int character_code;                                         //holds the cipher number for charachter
    char* cipher_buffer;                                        //holds cipher numbers
    int i;                                                      //clearly a loop counter..
    
    memset(input_buffer,'\0',1000);                             //clear input buffer in case it has garbage
    
    while((receiveno = read(sockfd,input_buffer,999)) > 0 && not_EndLine == true)                        //recieve through socket
    {                                                                                                       //sets up a array of int ascii values
        if (receiveno < 0)                                      //if nothing recieved output error
        {
            error("Error receiving from socket.");
        }
        
        
        
        for (i = 0; i < receiveno; i++)                                 //for all items received
        {
            
            if(input_buffer[i] == '\n')              //if an endline charachter is reached break the loop and take the next reception
            {
                not_EndLine = false;
                break;
            }
            else
            {
                
                if(input_buffer[i] == ' ')          //if space is found set up to hardcode a space in key
                {
                    character_code = 26;
                }
                else                                           //set up to store all alpha charachters in key buffer
                {
                    character_code = input_buffer[i] - 'A';
                }
                
                key_buffer[key_size] = character_code;         //store current charachter in key buffer
                key_size++;
            }
            
        }
    }
    
    sleep(1);                                                               //sleep for race condition
    
    memset(input_buffer, '\0' ,1000);                                       // again clear out input buffer
    
    while((receiveno = read(sockfd,input_buffer,255)) > 0) {                //follows same process as above to set up the text buffer
        //sets up a array of int ascii values
        
        if (receiveno < 0)
        {
            error("Error receiving from socket");
        }
        
        for(i = 0; i < receiveno; i++) {
            
            if(input_buffer[i] == '\n')
            {
                not_EndLine = true;
                break;
            }
            else if (input_buffer[i] == '\0')
            {
                not_EndLine = false;
                break;
            }
            
            else
            {
                
                if(input_buffer[i] == ' ')
                {
                    
                    character_code = 26;
                }
                else
                {
                    character_code = input_buffer[i] - 'A';
                }
                
                
                text_buffer[text_size] = character_code;
                text_size++;
            }
        }
    }
    
    cipher_buffer = malloc((sizeof(char *) * text_size) + 2);              //allocates memory for ciper + 2 because it will hold an endline and null charachter at end
    
    
    if(key_size < text_size)                                            //handles key being too short for text
    {
        error("Error: key is too short.");
    }
    
    
    
    for (i = 0; i < text_size; i++)                                            //loop through all of text
    {
        int key = key_buffer[i];                                    //current key value
        int txt = text_buffer[i];                                   //current text value
        
        key += txt;                                                 //key = key value + text value
        
        key = (key % 27);                                           //key = key % charcaters available in alpha
        // this sets the key for an abstracted value of the alpha
        
        if(key == 26)                                               //if its a space hardcode the space
        {
            cipher_buffer[i] = ' ';
        }
        else                                                        //otherwise set the cipher to the modulated char value chosen by the key
        {
            char cipher = key + 'A';
            cipher_buffer[i] = cipher;
        }
        
    }
    
    cipher_buffer[text_size] = '\0';                                //end the cipher line with an \n necessary for file handling
    
    sendno = write(sockfd, cipher_buffer,strlen(cipher_buffer));   //send the cipher off to the socket
    
    if (sendno < 0)                                                 //handle the error if it didnt work
    {
        error("Error sending to socket.");
    }
    
}

void decrypt_function(int sockfd) {
    
    //printf("ive reached decrypt!!!!!!!!!!!!!!");
    
    ssize_t receiveno,                                          //receive fd
    sendno;                                             //send fd
    int  key_size = 0;                                          //size of key
    int text_size = 0;                                          //size of text to encrypt
    bool not_EndLine = true;                                      //this is a bool for checking endline
    char input_buffer[1024];
    int* key_buffer = malloc(sizeof(int) * 70000);                //holds key
    int* cipherText_buffer = malloc(sizeof(int) * 70000);              //holds plain text
    int character_code;                                         //holds the cipher number for charachter
    char* plainText_buffer;                                        //holds cipher numbers
    int i;                                                      //clearly a loop counter..
    
    memset(input_buffer,'\0',1024);                             //clear input buffer in case it has garbage
    
    while((receiveno = read(sockfd,input_buffer,1023)) > 0 && not_EndLine == true)                        //recieve through socket
    {                                                                                                       //sets up a array of int ascii values
        if (receiveno < 0)                                      //if nothing recieved output error
        {
            error("Error receiving from socket.");
        }
        
        
        for (i = 0; i < receiveno; i++)                                  //for all items received
        {
            
            if(input_buffer[i] == '\n')              //if an endline charachter is reached break the loop and take the next reception
            {
                not_EndLine = false;
                break;
            }
            
            else
            {
                
                if(input_buffer[i] == ' ')          //if space is found set up to hardcode a space in key
                {
                    character_code = 26;
                }
                else                                           //set up to store all alpha charachters in key buffer
                {
                    character_code = input_buffer[i] - 'A';
                }
                
                key_buffer[key_size] = character_code;         //store current charachter in key buffer
                key_size++;
            }
            
        }
        
    }
    
    sleep(1);                                                               //sleep for race condition
    
    memset(input_buffer, '\0' ,1024);                                       // again clear out input buffer
    
    while((receiveno = read(sockfd,input_buffer,255)) > 0) {                //follows same process as above to set up the text buffer
        //sets up a array of int ascii values
        
        if (receiveno < 0)
        {
            error("Error receiving from socket");
        }
        
        for(i = 0; i < receiveno; i++) {
            
            if(input_buffer[i] == '\n')
            {
                break;
            }
            else if (input_buffer[i] == '\0')
            {
                break;
            }
            else
            {
                
                if(input_buffer[i] == ' ')
                {
                    
                    character_code = 26;
                }
                else
                {
                    character_code = input_buffer[i] - 'A';
                }
                
                cipherText_buffer[text_size] = character_code;
                text_size++;
            }
        }
    }
    
    plainText_buffer = malloc((sizeof(char *) * text_size) + 2);              //allocates memory for plain text + 2 because it will hold an endline and null charachter at end
    
    
    if(key_size < text_size)                                            //handles key being too short for text
    {
        error("Error: otp_dec cannot use otp_enc_d.");
    }
    
    
    for (i = 0; i < text_size; i++)                                          //loop through all of cipher
    {
        int key = key_buffer[i];                                    //current key value
        int cipher_value = cipherText_buffer[i];                     //current cipher value
        
        cipher_value -= key;                                         //cipher_value = key value + text value
        
        if (cipher_value < 0)                                       //reorder cipher value to alpha
        {
            cipher_value = cipher_value + 27;
        }
        else                                                        //otherwise set the cipher to the modulated char value chosen by the key
            
        {
            cipher_value = (cipher_value % 27);
        }
        
        if(cipher_value == 26)                                               //if its a space hardcode the space
        {
            plainText_buffer[i] = ' ';
        }
        else                                                        //set char to plain text array
        {
            char plain_text = cipher_value + 'A';
            plainText_buffer[i] = plain_text;
        }
        
    }
    
    sendno = write(sockfd, plainText_buffer,strlen(plainText_buffer));   //send the plain text off to the socket
    
    if (sendno < 0)                                                       //handle the error if it didnt work
    {
        error("Error sending to socket.");
    }
    
}



#endif 
