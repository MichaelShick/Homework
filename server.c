#include "chat.h"

client *clients;         // dynamic clients array
int localSocket;         // server socket
int clientsIn = 0;       // clients joined - for ID setup
int run = 1;             // run the server?
struct sockaddr_in serv; // server parms
pthread_mutex_t lock;

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
void sendMessage(msg message, int socket)
{
}
void passMessage(msg recviedMessage, int sender) // pass message to everyone in the server except the sender socket
{
    pthread_mutex_lock(&lock);
    int i;
    for (i = 0; i < clientsIn; i++)
    {
        if ((clients + i)->socket != sender)
        {
            printf("passed to socket -- %s\n", recviedMessage.content);
            printf("sent %d bytes!\n", exitOnError(send(
                                                       (clients + i)->socket, &recviedMessage, MAX_MESSAGE_LEN, 0),
                                                   "Send2\n"));
        }
    }
    pthread_mutex_unlock(&lock);
}
void printClient(client *cl)
{
    printf("client address pointer = %p client no.%d socket.%d on thread.%lu \n", cl, cl->clientID, cl->socket, cl->threadID);
    fflush(stdout);
}
void printAllClients()
{
    int i;
    pthread_mutex_lock(&lock);
    for (i = 0; i < clientsIn; i++)
    {
        printClient(clients + i);
    }
    pthread_mutex_unlock(&lock);
}
// must always have one free space for new clients, returning the new Client
// on success
client addClient(int socket, pthread_t threadID)
{

    pthread_mutex_lock(&lock);
    clients = realloc(clients, sizeof(client) + (sizeof(client) * clientsIn));
    (clients + clientsIn)->clientID = clientsIn;
    (clients + clientsIn)->socket = socket;
    // (clients + clientsIn)->threadID = threadID;
    fflush(stdout);
    exitOnError(send(
                    socket, "===WELCOME TO THE TALKING BEN===", MAX_MESSAGE_LEN, 0),
                "Send1\n");
    exitOnError(recv(socket, ((clients + clientsIn)->nickname), MAX_NAME_LEN, 0), "recv()");
    //   thread id must be passed from pthread_create()
    if (clients == NULL)
    {
        pthread_mutex_unlock(&lock);
        exitOnError(-1, "memory allocation");
    }
    // printClient(clients+clientsIn);
    pthread_mutex_unlock(&lock);
    clientsIn++;
    return *(clients + (clientsIn - 1));
}
// remove the client with the specified socket from the dynamic array
void removeClient(int socket)
{
    client tmp;
    msg leaveMsg;
    printAllClients();
    int i;
    //pthread_mutex_lock(&lock);
    for (i = 0; (clients + i)->socket != socket; i++)
    {
    };
    tmp = *(clients + (clientsIn - 1));
    // todo put lines 103-107 into a USERLEFT funciton
    strncpy(leaveMsg.name, "SERVER", 7);
    strncpy(leaveMsg.content, tmp.nickname, (strlen(tmp.nickname)));
    strncpy((leaveMsg.content + strlen(tmp.nickname)), "has left the chat room\0", 24);
    passMessage(leaveMsg, socket);
    *(clients + i) = tmp;
    clientsIn--;
    clients = realloc(clients, sizeof(client) + (clientsIn * sizeof(client)));
    printAllClients();
    //pthread_mutex_unlock(&lock);
}
int waitForMsg(client *cl)
{
    int i;
    msg buffer;
    int val = exitOnError(recv(cl->socket, &buffer, MAX_MESSAGE_LEN, 0), "recv()");
    // buffer[val] = 0;
    //  if (strcmp(buffer, "!exit"))
    //  printf("the message is %s\n", buffer);
    if (val == -1)
    {
        printf("error?");
        // TODO close connection
        return NULL;
    }
    else if (val == 0)
    {
        if (send(cl->socket, "a", 1, 0) == -1)
        {
            printf("no message\n");
            fflush(stdout);
            removeClient(cl->socket);
            return -1;
        }
        fflush(stdout);
    }
    else if (val > 0)
    {
        fflush(stdout);
        passMessage(buffer, cl->socket);
        fflush(stdout);
    }

    // sleep(5);
    fflush(stdout);
    return 1;
}
void *clientHandler(void *vargp)
{
    int socketBuffer = *(int *)vargp;
    client localClient = addClient(socketBuffer, pthread_self());
    // printClient(&localClient);
    while (1)
    {
        if (waitForMsg(&localClient) == -1)
        {
            pthread_exit(NULL);
        }
    }
}

void main()
{
    int addrlen;
    int socketBuffer;    // for storing connected sockets until assigned to struct
    client clientBuffer; // same as above but for clients

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(1);
    }

    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL); // to disable error no.141   on send to closed socket
    clients = (client *)malloc(sizeof(client));
    exitOnError(clients == 0 ? -1 : 1, "clients allocation");
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(PORT);
    localSocket = socket(AF_INET, SOCK_STREAM, 0);

    exitOnError(bind(localSocket, (struct sockaddr *)&serv, sizeof(serv)), "bind()");
    printf("address %d binded succesfully!\n", PORT);

    exitOnError(listen(localSocket, MAX_CLIENTS), "listen()");
    printf("listening...\n");

    fflush(stdout);
    while (run)
    {
        pthread_t num;
        addrlen = sizeof(serv);
        socketBuffer = exitOnError((accept(localSocket, (struct sockaddr *)&serv, &addrlen)), "accept()");
        printf("accepted %d\n", socketBuffer);
        fflush(stdout);
        // printClient(&clientBuffer);
        pthread_create(&num, NULL, (void *)clientHandler, &(socketBuffer));
        printf("clients in : %d\n", clientsIn);
        fflush(stdout);
    }
}