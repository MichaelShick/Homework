#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include "chat.h"

int exitOnError(int status, char *str)
{
    if (status == -1)
    {
        printf("error - %s in %s\n", strerror(errno), str);
        fflush(stdout);
        exit(1);
    }
    return status;
}

void *serverListener(void *num)
{
    int i = 0;
    int recvSize;
    int serverSocket = *(int *)num;
    msg reciveBuffer;

    printf("Connected with socket %d!\n", serverSocket);
    while (1)
    {
        fflush(stdout);
        recvSize = exitOnError(recv(serverSocket, &reciveBuffer, sizeof(reciveBuffer), 0),"recv()");
        fflush(stdout);
        if (recvSize == 0)
        {
            if ((send(serverSocket, "", 1, 0)) == -1)
                ;
            printf("server is closed!\n");
            exit(1);
        }
        if (recvSize > 0)
        {
            printf("%s -->", reciveBuffer.name);
            puts(reciveBuffer.content);
            fflush(stdout);
        }
    }
    exit(1);
}
void main()
{
    int serverSocket; // will point to the results
    struct sockaddr_in serv;
    int i = 0;
    pthread_t threadId;
    msg buffer;

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv.sin_port = htons(PORT);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    exitOnError(connect(serverSocket, (struct sockaddr *)&serv, sizeof(serv)) < 0,"connect()");
    printf("the socket is %d\n", serverSocket);
    fflush(stdout);
    exitOnError(recv(serverSocket, &buffer.content, MAX_MESSAGE_LEN, 0),"recv()");
    printf("%s\n", buffer.content);
    printf("enter your name :");
    fflush(stdout);
    scanf("%31s", buffer.name);
    while(!strcmp(buffer.name,"SERVER") && !strcmp(buffer.name,"You"))
    {
    printf("Invalid name, try again:");
    fflush(stdout);
    scanf("%31s", buffer.name);
    }
    exitOnError(send(serverSocket, &buffer, MAX_NAME_LEN, 0),"send()");
    pthread_create(&threadId, NULL, serverListener, &serverSocket);
    fflush(stdout);

    while (1)
    {
        printf("You --> ", buffer.name);
        fflush(stdout);
        gets(buffer.content);
        exitOnError(send(serverSocket, &buffer, MAX_MESSAGE_LEN, 0),"send()");
    }
    // printf("the socket is %d\n",serverSocket);
    //   memset(msg, 0, sizeof(msg));
}