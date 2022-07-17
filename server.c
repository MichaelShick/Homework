#include "server.h"
#define _CRT_SECURE_NO_WARNINGS
clientPtr_s clients;      // dynamic clients array - fills as new sockets connect
pthread_mutex_t lock_g;   // mutex lock
int clients_in_g = 0;     // amount of clients connected
int threadsRunning_g = 0; // amount of threads currently running, to add threads to *threads

int exit_on_error(int status, char *str)
{
    if (status == -1)
    {
        printf("error - %s in %s\n", strerror(errno), str);
        exit(1);
    }
    else if (status < -1)
    {
        printf("error - error code no.%d\n", status);
        exit(1);
    }
    return status;
}
int socket_is_open(int socket)
{
    int closed = 0;
    msg_s magic_string = {"SERVERCHECKLMAO", 0};
    printf("%s\n", magic_string.name_t);
    if (send(socket, &magic_string, sizeof(magic_string), 0) == -1)
    {
        printf("is closed\n");
        closed = -1;
    }
    return closed;
}

void print_client(client_s *cl)
{
    printf("client address pointer = %p client no.%d socket.%d on thread.%lu\n", cl, cl->clientID_t, cl->socket_t, cl->threadID_t);
    fflush(stdout);
}
void print_all_clients()
{
    int i;
    pthread_mutex_lock(&lock_g);
    for (i = 0; i < clients_in_g; i++)
    {
        print_client(clients + i);
    }
    pthread_mutex_unlock(&lock_g);
}
void pass_message(msg_s recviedMessage, int sender)
{
    int i;
    for (i = 0; i < clients_in_g; i++)
    {
        if ((clients + i)->socket_t != sender)
        {

            if (socket_is_open(clients[i].socket_t) == -1)
            {
                close(clients[i].socket_t);
            }
            else
            {
                printf("passed to socket -- %s\n", recviedMessage.content_t);
                printf("sent %d bytes!\n", exit_on_error(send(
                                                             (clients + i)->socket_t, &recviedMessage, MAX_MESSAGE_LEN, 0),
                                                         "Send2\n"));
            }
        }
    }
}
void add_client(int socket, pthread_t threadID, clientPtr_s tmp)
{
    pthread_mutex_lock(&lock_g);
    msg_s buffer;
    clients = realloc(clients, sizeof(client_s) + (sizeof(client_s) * (clients_in_g + 1)));
    if (clients == NULL)
    {
        pthread_mutex_unlock(&lock_g);
        exit_on_error(-1, "memory allocation");
    }
    clients[clients_in_g].clientID_t = clients_in_g;
    clients[clients_in_g].socket_t = socket;
    clients[clients_in_g].threadID_t = threadID;
    if (socket_is_open(socket) == 0)
    {
        printf("sending welcome to %d\n", socket);
        // every client must trade the welcome signal for his nickname
        strncpy(buffer.name_t, "SERVER", 7);
        strncpy(buffer.content_t, "===WELCOME TO THE TALKING BEN===", 33);
        printf("%s : %s\n", buffer.name_t, buffer.content_t);
        exit_on_error(send(
                          socket, &buffer, MAX_MESSAGE_LEN, 0),
                      "Send1\n");
        exit_on_error(recv(socket, &buffer, MAX_NAME_LEN, 0), "recv()");
        //   thread id must be passed from pthread_create()
        strncpy(clients[clients_in_g].nickname_t, buffer.name_t,MAX_NAME_LEN);
        printf("clients_in_g %d\n",clients_in_g);
        *tmp = clients[clients_in_g];
        clients_in_g++;
    }
    pthread_mutex_unlock(&lock_g);
}
void removeClient(int socket)
{
    printf("removing with old size %d\n", clients_in_g);
    client_s tmp;
    msg_s leaveMsg;
    print_all_clients();
    int i;
    if (clients_in_g == 1)
    {
        printf("only one!\n");
        clients_in_g--;
        clients == NULL;
    }
    else
    {

        //pthread_mutex_lock(&lock_g);
        for (i = 0; clients[i].socket_t != socket; i++)
        {
        };
        tmp = clients[clients_in_g - 1];
        memset(leaveMsg.name_t, 0, MAX_NAME_LEN);
        memset(leaveMsg.content_t, 0, MAX_MESSAGE_LEN);
        strncpy(leaveMsg.name_t, "SERVER", 7);
        strncpy(leaveMsg.content_t, tmp.nickname_t, (strlen(tmp.nickname_t)));
        strncpy((leaveMsg.content_t + strlen(tmp.nickname_t)), "has left the chat room\0", 24);
        pass_message(leaveMsg, socket);

        clients[i] = tmp;
        clients_in_g--;
        clients = realloc(clients, sizeof(client_s) + (clients_in_g * sizeof(client_s)));
        if (clients == NULL)
        {
            exit_on_error(-1, "clients realloc()");
        }
    }
    print_all_clients();
    //pthread_mutex_unlock(&lock_g);
}
int wait_for_msg(clientPtr_s cl)
{
    int i;
    msg_s buffer;
    if (socket_is_open(cl->socket_t) != 0)
    {
        printf("no message\n");
        removeClient(cl->socket_t);
        return -1;
    }
    int val = recv(cl->socket_t, &buffer, MAX_MESSAGE_LEN, 0);
    if (val == -1)
    {
        printf("error?");
        removeClient(cl->clientID_t);
        close(cl->socket_t);
        return -1;
    }
    else if (val == 0)
    {
        if (socket_is_open(cl->socket_t) != 0)
        {
            printf("no message\n");
            removeClient(cl->socket_t);
            close(cl->socket_t);
            return -1;
        }
    }
    else if (val > 0)
    {
        pass_message(buffer, cl->socket_t);
    }
    // sleep(5);
    return 1;
}
void *clientHandler(void *vargp)
{
    printf("created\n");
    int socketBuffer = *(int *)vargp;
    clientPtr_s localClient = malloc(sizeof(client_s));
    if(localClient == NULL)
    {
        printf("memory problem\n");
        exit(1);
    }
    add_client(socketBuffer, pthread_self(), localClient);
    // if add_client failed to fill
    if (localClient == NULL)
    {
        puts("failed to add a client!\n");
        close(socketBuffer);
        pthread_exit(NULL);
    }
    while (FOREVER)
    {
        // wait_for_msg should handle socket closure and client removal
        if (wait_for_msg(localClient) == -1)
        {
            threadsRunning_g--;
            printf("EXITING!!\n");
            pthread_exit(NULL);
        }
    }
}
int socket_bind_listen(struct sockaddr_in *info)
{
    int socketBuffer;
    // socket
    socketBuffer = exit_on_error(socket(info->sin_family, SOCK_STREAM, 0), "socket()");
    // bind
    exit_on_error(bind(socketBuffer, info, sizeof(struct sockaddr_in)), "bind()");
    puts("binded succesfuly!");

    // listen
    exit_on_error(listen(socketBuffer, 20), "listen()");
    puts("listening...");

    return socketBuffer;
}
void init_addrinfo(struct addrinfo *info)
{
    struct addrinfo setup; // params setup
    setup.ai_family = AF_INET;
    setup.ai_socktype = SOCK_STREAM;
    // setup.ai_protocol = NULL;
    setup.ai_flags = AI_PASSIVE;
    printf("%s", PORT);
    exit_on_error(getaddrinfo(NULL, PORT, &setup, &info), "getadrrInfo()");
}
void init_sockaddr(struct sockaddr_in *params)
{
    params->sin_family = AF_INET;
    params->sin_addr.s_addr = INADDR_ANY;
    params->sin_port = htons(PORT);
}
void init_server()
{
    struct sockaddr_in params;
    int paramsLen;
    int socketBuffer;      // for storing connected sockets until assigned to struct
    int serverSocket;      // the socket of the server
    client_s clientBuffer; // same as above but for clients
    pthread_t threads;

    threads = malloc(sizeof(pthread_t) * MAX_CLIENTS);
    clients = malloc(sizeof(client_s));

    exit_on_error((clients == NULL || threads == NULL) ? -1 : 0, "clients/threads malloc"); // try to malloc
    exit_on_error(pthread_mutex_init(&lock_g, NULL), "mutexInit()");

    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL); // to disable error no.141   on

    init_sockaddr(&params);
    serverSocket = socket_bind_listen(&params);
    paramsLen = sizeof(params);
    do
    {
        while (threadsRunning_g < MAX_CLIENTS)
        {

            puts("waiting for accept");
            socketBuffer = exit_on_error((accept(serverSocket, (struct sockaddr *)&params, &paramsLen)), "accept()");
            printf("accepted %d\n", socketBuffer);
            exit_on_error(pthread_create(threads + threadsRunning_g++, NULL, (void *)clientHandler, &(socketBuffer)), "thread creation");
        }

        puts("max clients reached, going to sleep..");
        // sleep(10);
    } while (FOREVER);

    close(serverSocket);
    free(clients);
    free(threads);
    pthread_mutex_destroy(&lock_g);
    freeaddrinfo(&params);
}
int main()
{
    setbuf(stdout, NULL);

    init_server();

    return 1;
}
