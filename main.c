#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "my_header.h"
#define SIZE 4096
#define STR_SIZE 80

int main(int argc, char *argv[]){
    char buffer[SIZE];          // Creating buffer to store data readed from file.
    char* filePath = argv[2];   // Path of file that is given on the last command line argument
    int readedBytes;            // Number of readed bytes
    int fdRead, fdWrite;        // Directory stream file descriptors for reading and writing
    if(argc != 3)
        printErrorAndExit();
    
    // OPENING FILE ON READ ONLY MODE
    if( (fdRead = open(filePath, O_RDONLY, S_IWGRP)) == -1 ){
        perror("Error while opening the file to read.\n");
        exit(2);
    }

    // READING FILE AND SAVING CONTENT TO BUFFER
    while( (readedBytes = read(fdRead, buffer, SIZE)) == -1 && errno == EINTR ){ /* Intentionanlly Empty loop to deal interruptions by signal */}
    if(readedBytes <= 0){
        perror("File is empty. Goodbye\n");
        exit(3);
    }
    
    // READING IS COMPLETED. CLOSING THE FILE
    if( close(fdRead) == -1 ){   
        perror("Error while closing the file.");
        exit(4);
    }

    // OPENING THE FILE AGAIN [THIS TIME IN WRITE MODE]
    if( (fdWrite = open(filePath, O_WRONLY, S_IWGRP)) == -1 ){
        perror("Error while opening the file to write.\n");
        exit(5);
    }

    // PARSING OPERATIONS FROM COMMAND LINE ARGUMENT TO AN ARRAY
    char** operations = argDivider( argv[1] );
    int size = sizeof(operations) / sizeof(operations[0]);
    printf("Size is %d\n", size); //çç

    //  TO THE TEMPORARY FILE
    exchanger(buffer, operations, size);
    // IF OPERATIONS FOUND ON FILE, BUFFER IS CHANGED

    // WRITING THE NEW BUFFER INTO THE SAME INPUT FILE
    while( write(fdWrite, buffer, strlen(buffer)) == -1 && errno == EINTR ){/* Intentionanlly Empty loop to deal interruptions by signal */}
    printf("Succesfully writed to the file\n");

    // WRITING IS COMPLETED. CLOSING THE FILE.
    if( close(fdWrite) == -1 ){   
        perror("Error while closing the file.");
        exit(6);
    }
    return 0;
}