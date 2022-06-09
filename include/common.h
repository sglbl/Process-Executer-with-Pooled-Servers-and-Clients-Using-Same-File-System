#ifndef COMMON_H_
#define COMMON_H_

/*
	@author Suleyman Golbol
	@number 1801042656
*/

#define TRUE 1
#define FALSE 0

typedef struct String {
    char *data;
} String;

typedef struct SgLinkedList{
    String string;
    struct SgLinkedList *next;
} SgLinkedList;

/* Returns time stamp */
char *timeStamp();
/* Prints error message and exits */
void errorAndExit(char *errorMessage);

#endif