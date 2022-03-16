#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include "sg_replacer.h"
#define TRUE 1
#define FALSE 0
#define STR_SIZE 80

void printErrorAndExit(){
    perror( "ERROR FOUND ON ARGUMENTS; PLEASE ENTER A VALID INPUT! INSTRUCTIONS:\n"
    "./hw1 '/str1/str2/' inputFilePath -> For just to replace\n"
    "./hw1 '/str1/str2/i' inputFilePath ->Insensitive replace\n"
    "./hw1 '/str1/str2/i;/str3/str4/' inputFilePath -> Multiple replacement operations\n"
    "./hw1 '/[zs]tr1/str2/' inputFilePath -> Multiple character matching like this will match both ztr1 and str1\n"
    "./hw1 '/^str1/str2/' inputFilePath -> Support matching at line starts like this will only match lines starting with str1 \n"
    "./hw1 '/str1$/str2/' inputFilePath -> Support matching at line ends like this will only match lines ending with str1 \n"
    "./hw1 '/st*r1/str2/' inputFilePath -> Support 0 or more repetitions of characters like this will match sr1, str1, sttr1 \n"
    "./hw1 '/^Window[sz]*/Linux/i;/close[dD]$/open/' inputFilePath -> it supports arbitrary combinations of the above \n");
    printf("Goodbye!\n");
    exit(EXIT_FAILURE);
}

char** argDivider(char* arg, int *counter){
    int i;
    if( strncmp(arg, "/", 1) != 0){  // If first letter of argument is not '/' exit.
        printErrorAndExit();
    }

    //Finding number of operations
    int numberOfOperations = 1;
    for(i = 0; i < strlen(arg); i++){
        if(arg[i] == ';') 
            numberOfOperations++;  
    }

    char** operations = (char**)calloc(numberOfOperations, sizeof(char));
    char *token = strtok(arg, ";");
    for(i = 0; token != NULL && i < numberOfOperations; i++){
        operations[i] = (char*)calloc(20, sizeof(char));
        operations[i] = token;
        token = strtok(NULL, ";");
    }
    *counter = i;
    
    return operations;
}

void replacer(char* buffer, char** operations, int size){
    // For every operation (that's divided by / symbols on argument )
    for(int i=0; i<size; i++){    
        char* tempOperation = (char*)calloc(strlen(operations[i]), sizeof(char));
        strcpy(tempOperation, operations[i]);                                 
        ReplaceMode mode = SENSITIVE;

        printf("Operation is %s\n", operations[i] );
        // Parsing operations[i] to get the first argument of operation
        char *str1 = strtok(tempOperation, "/");
        // Parsing operations[i] again to get the second argument of operation
        char *str2 = strtok(NULL, "/");
        // Check if argumant contains 'i', if contains it's insensitive
        if( strtok(NULL, "/") != NULL ){
            mode = INSENSITIVE;
        }
        
        // This function uses bitwise | operator in order to select replace mode in an easier way.
        if(str1[0] == '^'){        // if first argumant after / has ^ then it means it should support matching at line starts
            mode |= LINE_START;
            printf("LINE MODE IS %d\n", mode);
            str1++; //Moving string one char right so get rid of ^
        }
        if(str1[strlen(str1) - 1] == '$'){  // if first argumant has $ then it means it should support matching at line ends
            printf("line end supporTTTT\n");
            printf("Mode1 is %d\n", mode);
            mode |= LINE_END;
            printf("Mode2 is %d\n", mode);
            str1[ strlen(str1) - 1 ] = '\0'; // Truncate argument by 1 [Removing $ sign from the end]
        }
        
        char keyValue;
        char* afterAsterisk;
        if( (afterAsterisk = strchr(str1, '*')) != NULL ){  // If argument is "st*r7"
            mode |= REPETITION;
            printf("After asteriks is %s\n", afterAsterisk);
            keyValue = (afterAsterisk-1)[0];
            printf("Key value is %c", keyValue);
            //for(int j=0; str1[j] != keyValue; j++);
            printf("SIZE is %ld", strlen(str1) - strlen(afterAsterisk) - 1);

            printf("--After asteriks is %s\n", --afterAsterisk);
            printf("Str1 is %s\n", str1);
        }

        // If operation contains [ and ] characters than it supports multiple 
        if(strchr(operations[i], '[') != NULL && strchr(operations[i], ']') != NULL){
            printf("xx");
            multipleReplacer(buffer, ++operations[i], mode);  // Changing cursor 1 to the right so get rid of '/' symbol
            continue;
        }

        printf("STR1-> %s | STR2-> %s | MODE-> %d \n", str1, str2, mode);
        replace(buffer, str1, str2, mode);                         // Replacing
    }

}

void replace(char* buffer, char *str1, char *str2, ReplaceMode mode){
    int bufferSize = strlen(buffer);
    StrInfo str1Info; //To hold size and index 
    str1Info.index = -1;

    // Getting index of str1 (argument 1) to replace
    for(int i=0; i<bufferSize; i++){
        if( mode == SENSITIVE ){
            if( strncmp(buffer+i, str1, strlen(str1) ) == 0){
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == INSENSITIVE ){
            if( strncasecmp(buffer+i, str1, strlen(str1) ) == 0){
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (SENSITIVE | LINE_START) ){
            if( (buffer+i-1)[0] == '\n' && strncmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                printf("LINEEE");
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (INSENSITIVE | LINE_START) ){
            if( ( (buffer+i-1)[0] == '\n' || i == 0)  // If it's first line; then it means that it's first line of file or the previous char is '\n'
                && strncasecmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (SENSITIVE | LINE_END) ){
            if( ( ((buffer + strlen(str1) + i)[0] == '\n') || ((buffer + strlen(str1) + i)[0] == '\0') ) // If next char is '\n' or EOF then it supports $
                && strncmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (INSENSITIVE | LINE_END) ){
            if( ( ((buffer + strlen(str1) + i)[0] == '\n') || ((buffer + strlen(str1) + i)[0] == '\0') ) // If next char is '\n' or EOF then it supports $
                && strncasecmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }

        else if( mode == (SENSITIVE | LINE_START | LINE_END) ){
            if( ( (buffer+i-1)[0] == '\n' || i == 0) // If it's first line; then it means that it's first line of file or the previous char is '\n' 
                && ( ((buffer + strlen(str1) + i)[0] == '\n') || ((buffer + strlen(str1) + i)[0] == '\0') ) // If next char is '\n' or EOF then it supports $
                && strncmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (INSENSITIVE | LINE_START | LINE_END) ){
            if( ( (buffer+i-1)[0] == '\n' || i == 0) // If it's first line; then it means that it's first line of file or the previous char is '\n' 
                && ( ((buffer + strlen(str1) + i)[0] == '\n') || ((buffer + strlen(str1) + i)[0] == '\0') ) // If next char is '\n' or EOF then it supports $
                && strncasecmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        else if( mode == (SENSITIVE | REPETITION) ){
            printf("REPETITION MODE ON");
            if( ( ((buffer + strlen(str1) + i)[0] == '\n') || ((buffer + strlen(str1) + i)[0] == '\0') ) // If next char is '\n' or EOF then it supports $
                && strncasecmp(buffer+i, str1, strlen(str1) ) == 0){ //Checking if starting of a line
                str1Info.index = i;
                str1Info.size = strlen(str1);
            }
        }
        

        if(str1Info.index  != -1){
            for(int j=0; j < str1Info.size; j++)
                buffer[str1Info.index + j] = str2[j]; //Changing info in buffer
        }
        
    }

    if(str1Info.index  == -1){
        printf("WARNING! %s couldn't be found on file.\n", str1);
        return;
    }

    printf("New buffer is '%s'\n", buffer);
}

void multipleReplacer(char* buffer, char* operation, ReplaceMode mode){
    char* string;
    int leftSqIndex, rightSqIndex;
    
    char* arg1 = strtok(operation, "/");
    int size = strlen(arg1);
    char *str2 = strtok(NULL, "/");

    if(mode == (SENSITIVE | LINE_START) || mode == (SENSITIVE | LINE_START | LINE_END) || mode == (INSENSITIVE | LINE_START) || mode == (INSENSITIVE | LINE_START | LINE_END) )
        arg1++; // If it is a line start operation (^), then we need to increment it to shift right

    // Finding how many MULTIPLE operations are there.
    for(leftSqIndex=0; arg1[leftSqIndex] != '['; ++leftSqIndex); 
    for(rightSqIndex=leftSqIndex; arg1[rightSqIndex] != ']'; ++rightSqIndex); 

    for(int i = 1; i < (rightSqIndex - leftSqIndex); i++){
        string = (char*)calloc(size, sizeof(char));         // For example if first argument is S[TL]R1 
        strncat(string, arg1, leftSqIndex);            // Adding S to string  
        strncat(string, arg1 + leftSqIndex + i, 1);    // Adding T and L to string in different cycles of loop
        strncat(string, arg1 + rightSqIndex + 1, size - (rightSqIndex+1) ); // Adding R1 (adding rest of it to string).
        replace(buffer, string, str2, mode);
    }

}