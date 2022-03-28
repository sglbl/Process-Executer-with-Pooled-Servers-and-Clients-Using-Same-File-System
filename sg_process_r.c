#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // For memset() function for the locking mechanism
#include <errno.h> // spesific error message
#include <fcntl.h> // provide control over open files
#include <unistd.h> // unix standard functions
#include "sg_process_r.h"
#include "sg_matrix.h"

extern char **environ;  // Environment variables that will be passed to child process and will come from "sg_process_p.c"

int main(int argc, char *argv[]){
    int i = argv[1][0];       // Special number of this child.
    char *filePath = argv[3]; // Output file path
    int fileDesc;             // Directory stream file descriptor for file writing
    struct flock lock;        // Lock structure of the file.

    // Opening file in write mode
    if( (fileDesc = open(filePath, O_WRONLY | O_APPEND, S_IWGRP)) == -1 ){
        perror("Error while opening file to write.\n");
        exit(EXIT_FAILURE);
    }

    // Locking
    memset(&lock, 0, sizeof(lock)); //Initing structure of lock to 0.
    lock.l_type = F_WRLCK;  // F_WRLCK: Field of the structure of type flock for write lock.
    if( fcntl(fileDesc, F_SETLKW, &lock) == -1 ){ // putting write lock on file. 
        // F_SETLKW: If a signal is caught while waiting, then the call is interrupted and (after signal handler returned) returns immediately.
        perror("Error while locking fcntl(F_SETLK mode).\n");
        exit(EXIT_FAILURE);
    }
    
    // Printing child info
    printChildInfo(i);
    double **covarianceMatrix = findCovarianceMatrix(i);

    // Writing to file
    writeToFile(fileDesc, covarianceMatrix);

    // Unlocking
    lock.l_type = F_UNLCK;
    if ( fcntl(fileDesc, F_SETLKW, &lock) == -1) {
        perror("Error while unlocking with fcntl(F_SETLKW)");
        exit(EXIT_FAILURE);
    }

    // Closing file
    if( close(fileDesc) == -1 ){   
        perror("Error while closing the file.");
        exit(EXIT_FAILURE);
    }

    return 0;
}

double** findCovarianceMatrix(int i){
    // Formula:        a = A – 1*A*( 1 / n )
    // Covariance matrix = a‘ * a / n
    // Reference: https://stattrek.com/matrix-algebra/covariance-matrix.aspx

    double **dataset = (double**)calloc( CHILD_SIZE, sizeof(double*) );
    double **tempMatrix = (double**)calloc( CHILD_SIZE, sizeof(double*) );
    for(int j = 0; j < CHILD_SIZE; j++){
        dataset[j] = (double*)calloc( COORD_DIMENSIONS, sizeof(double*) );
        tempMatrix[j] = (double*)calloc( COORD_DIMENSIONS, sizeof(double*) );
        for(int k = 0; k < COORD_DIMENSIONS; k++){
            dataset[j][k] = (double)environ[i][j*COORD_DIMENSIONS+ k];
        }
    }
    
    tempMatrix = matrixMultiplicationFor10x3(dataset);
    divide10x3MatrixTo10(tempMatrix);
    substract10x3Matrices(dataset, tempMatrix);

    double **covarianceMatrix;
    covarianceMatrix = multiplyWithItsTranspose(dataset);
    divide3x3MatrixTo10(covarianceMatrix);

    // Freeing tempMatrix because not needed anymore.
    for(int j = 0; j < CHILD_SIZE; j++){
        free(tempMatrix[j]);
    }
    free(tempMatrix);

    // Freeing dataset because not needed anymore.
    for(int j = 0; j < CHILD_SIZE; j++){
        free(dataset[j]);
    }
    free(dataset);

    // printf("Covariance matrix is \n");
    // for(int i = 0; i < 3; i++){
    //     for(int j = 0; j < 3; j++){
    //         printf("%.3f ", covarianceMatrix[i][j] );
    //     }
    //     printf("\n");
    // }

    return covarianceMatrix;
}

void writeToFile(int fileDesc, double **covarianceMatrix){
    // Size of covariance matrix is 3x3. 
    for(int j = 0; j < COORD_DIMENSIONS; j++){
        for(int k = 0; k < COORD_DIMENSIONS; k++){
            //Writing as binary.
            while( write(fileDesc, &covarianceMatrix[j][k], sizeof(covarianceMatrix[j][k]) ) == -1 && errno == EINTR ){}
        }
    }
}

/* Printing child information */
void printChildInfo(int i){
    // Because of printf and snprintf are not signal safe, I used write(). 
    // For formatting from int to string I used itaaForAscii(int) function.

    // Creating char* variables to free them after.
    char *childNum, *val00, *val01, *val02, *val10, *val11, *val12, *val90, *val91, *val92;

    write(STDOUT_FILENO, "Created R_", 11);
    write(STDOUT_FILENO, childNum = itoaForAscii(i+1), sizeof( itoaForAscii(i+1) ) );
    write(STDOUT_FILENO, " with (", 8);
    write(STDOUT_FILENO, val00 = itoaForAscii( environ[i][0] ), sizeof( itoaForAscii(environ[i][0]) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val01 = itoaForAscii( environ[i][1] ), sizeof( itoaForAscii(environ[i][1]) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val02 = itoaForAscii( environ[i][2] ), sizeof( itoaForAscii(environ[i][2]) ) );
    write(STDOUT_FILENO, "), (", 4);
    write(STDOUT_FILENO, val10 = itoaForAscii( environ[i][3] ), sizeof( itoaForAscii(environ[i][3]) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val11 = itoaForAscii( environ[i][4] ), sizeof( itoaForAscii(environ[i][4]) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val12 = itoaForAscii( environ[i][5] ), sizeof( itoaForAscii(environ[i][5]) ) );
    write(STDOUT_FILENO, "), ..., (", 10);
    write(STDOUT_FILENO, val90 = itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-3] ), sizeof( itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-3] ) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val91 = itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-2] ), sizeof( itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-2] ) ) );
    write(STDOUT_FILENO, ",", 1);
    write(STDOUT_FILENO, val92 = itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-1] ), sizeof( itoaForAscii( environ[i][CHILD_SIZE*COORD_DIMENSIONS-1] ) ) );
    write(STDOUT_FILENO, ")\n", 2);

    // Freeing
    free(val00); free(val01); free(val02);
    free(val10); free(val11); free(val12);
    free(val90); free(val91); free(val92);
    free(childNum);

}

char* itoaForAscii(int number){
    if(number == 0){
        char* string = calloc(2, sizeof(char));
        string[0] = '0';    string[1] = '\0';
        return string;
    }

    int digitCounter = 0;
    int temp = number;
    while(temp != 0){
        temp /= 10;
        digitCounter++;
    }
    
    char* string = calloc((digitCounter+1), sizeof(char));
    for(int i = 0; i < digitCounter; i++){
        char temp = (number % 10) + '0';
        string[digitCounter-i-1] = temp;
        number /= 10;
    }
    string[digitCounter] = '\0';
    
    return string;
}