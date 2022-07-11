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
#include <signal.h>

#define PORT 9003
#define MAX_CLIENTS 100
#define MAX_NAME_LEN 32
#define MAX_MESSAGE_LEN 128 + MAX_NAME_LEN

typedef struct msgPacket
{
    char name[MAX_NAME_LEN];
    char content[MAX_MESSAGE_LEN];
} msg;
typedef struct client
{
    int clientID;
    int socket; // client CONNECTED socket
    char nickname[MAX_NAME_LEN];
    pthread_t threadID; // each client has his own message recieving thread
} client;

int exitOnError(int status, char *str);
void *serverListener(void *num);
void sendMessage(msg message, int socket);
void passMessage(msg recviedMessage, int sender); // pass message to everyone in the server except the sender socket
void printClient(client *cl);
void printAllClients();
client addClient(int socket, pthread_t threadID);
void removeClient(int socket);
int waitForMsg(client *cl);
void *clientHandler(void *vargp); // thread for accepting and forwarding messages
