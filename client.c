
#include "client.h"

int exit_on_error(int status, char *str)
{
    if (status == -1)
    {
        printf("error - %s in %s\n", strerror(errno), str);
        exit(1);
    }
    return status;
}
/*
basiaclly a recv() extention - the function loops recieve until it recieves anything but
the magic string, and stores it into buffer.
int socket - the socket to call recv() on
msg_s *buffer - the content recieved
the function returns the recv() content by saving it into the buffer
*/
int magic_recv(int socket, msg_s *buffer)
{
    int flag = 1;
    int res = recv(socket, buffer, sizeof(msg_s), 0);
    while (res >= 0 && flag)
    {
        if (res > 0)
        {
            if (strncmp(buffer->name_t, "SERVERCHECKLMAO", 16) == 0)
            {
                res = recv(socket, buffer, sizeof(msg_s), 0);
            }
            else
            {
                flag = !flag;
            }
        }
        else if (res <= 0)
        {
            printf("something went wrong\n");
            close(socket);
            exit(1);
        }
    }
    printf("good\n");
    return res;
}
void *server_listener(void *socket)
{
    int i = 0;
    int recvSize;
    int servSocket = *(int *)socket;
    msg_s reciveBuffer;

    printf("Connected with socket %d!\n", servSocket);
    while (FOREVER)
    {
        recvSize = magic_recv(servSocket, &reciveBuffer);
        if (recvSize == 0)
        {
            if ((send(servSocket, "", 1, 0)) == -1)
                ;
            printf("server is closed!\n");
            close(socket);
            pthread_exit(1);
        }
        if (recvSize > 0)
        {
            printf("%s -->", reciveBuffer.name_t);
            puts(reciveBuffer.content_t);
        }
        else
        {
            printf("big bad\n");
            close(socket);
            pthread_exit(1);
        }
    }
    exit(1);
}
/*
set up sockaddr
sockaddr_in *params - the sockaddr to fill
doesn't return anything
*/
void init_sockaddr(struct sockaddr_in *params)
{
    params->sin_family = AF_INET;
    params->sin_addr.s_addr = INADDR_ANY;
    params->sin_port = htons(PORT);
}
void init_client()
{
    int servSocket;            // the socket to connect to
    struct sockaddr_in params; // socket params
    int i = 0;                 // index buffer
    pthread_t threadId;        // the client's thread ID
    msg_s buffer;              // buffer for sending\recieving messages

    setbuf(stdout, NULL);
    init_sockaddr(&params);
    servSocket = socket(AF_INET, SOCK_STREAM, 0);
    exit_on_error(connect(servSocket, (struct sockaddr *)&params, sizeof(params)), "connect()");
    printf("the socket is %d\n", servSocket);
    magic_recv(servSocket, &buffer);
    printf("%s\n", buffer.content_t);
    printf("enter your name :");
    gets(buffer.name_t);
    while (strcmp(buffer.name_t, "SERVER") == 0 || strcmp(buffer.name_t, "You") == 0 || strcmp(buffer.name_t, "SERVERCHECKLMAO") == 0)
    {
        printf("Invalid name, try again:");
        gets(buffer.name_t);
    }
    exit_on_error(send(servSocket, &buffer, MAX_NAME_LEN, 0), "send()");
    pthread_create(&threadId, NULL, server_listener, &servSocket);
    while (FOREVER)
    {
        printf("You --> ", buffer.name_t);
        gets(buffer.content_t);
        exit_on_error(send(servSocket, &buffer, MAX_MESSAGE_LEN, 0),"send()");
        
    }
    // printf("the socket is %d\n",servSocket);
    //   memset(msg_s, 0, sizeof(msg_s));
}
void main()
{
    init_client();
}