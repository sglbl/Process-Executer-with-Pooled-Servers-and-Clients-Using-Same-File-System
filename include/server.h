#ifndef SERVER_H_
#define SERVER_H_
/*
	@author Suleyman Golbol
	@number 1801042656
*/

/* Opens files */
void openFiles(char *filePath1, char *filePath2, char *output, int fileDescs[3]);
/* Reads matrices */
void readMatrices(int n, int m, int twoToN, int fileDescs[3], int matrixA[][twoToN], int matrixB[][twoToN]);
/* Creates threads */
void createThreads(int portNo);
/* Thread routine */
void *threadJob(void *arg);
/* Initializes signal handler */
void signalHandlerInitializer();
/* Signal handler */
void mySignalHandler(int signalNumber);
/* Stream socket communication with servant and client */
void tcpComm();
/* Main thread forwards incoming connections to threads */
void forwardIncomingConnection(int newServerSocketFd);

void addToQueue(int newFileDesc);

void removeFromQueue(int newFileDesc);

#endif