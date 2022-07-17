#define PORT 9001
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#define max_name_len
#define MAX_NAME_LEN 32
#define MAX_MESSAGE_LEN 128
#define MAX_CLIENTS 50
#define FOREVER 1

// mainly for storing connected sockets
typedef struct client
{
    int clientID_t;       // a clients uniqe ID
    pthread_t threadID_t; // each client is included in his own thread
    int socket_t;         // a pointer to the socket connecting the client
    char nickname_t[];    // a nickname to print with each user message
} client_s, *clientPtr_s;
// the default package to use with send() in both server.c and client.c
typedef struct msgPacket
{
    char name_t[MAX_NAME_LEN];       // name of the sender
    char content_t[MAX_MESSAGE_LEN]; // content to be sent
} msg_s, *msgPtr_s;
