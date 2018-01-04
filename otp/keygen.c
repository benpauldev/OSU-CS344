#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "otp_header.h"


int main(int argc, char *argv[]) {
    
    //Error checking for number of arguments passed to keygen.
    //requires that a key length value is passed as an argument with the keygen command.
    if (argc != 2)
    {
        printf("Usage: keygen requires length argument.\n");  //output to stderr
    }
    else
    {
        
        int i = 0;                      //loop counter
        int length;                 //length of key to generate
        length = atoi(argv[1]);     //takes the number from the command line and converts to int
        char key[length + 1];       //creates a key c-string size of passed value
        
        char values[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";      //key value string
        
        
        srand( (unsigned int) time(NULL) );                 //seed random generator
        
        while( i < length)
        {
            key[i] = values[rand() % 26];                   //initializes key array of random chars
            i++;
        }
        
        key[length] = '\0';                                 //null terminate the key
        
        printf("%s\n", key);                      //last character output is newline
    }
    return 0;
}
