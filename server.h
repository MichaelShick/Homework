#include "chat.h"
/*
exits in case that status is an error code, displaying it as well as an additional string
int status - the status to check
char *str - string to display
returns status in case it wasn't an error
*/
int exit_on_error(int status, char *str);
/*
pass an incoming message from sender to all the users save for the sender
msg_s recievedMessage - the message to pass
int sender - the client to ignore
doesn't return anything
*/
/*
check if the socket is open, call before any socket use
int socket - the socket to check
returns -1 on for closed socket\socket error, 0 otherwise
doesn't work properly, always returns 0. going to keep for later bugfix (or not)
*/
// int socket_is_open(int socket)
// {
// int bad = 0;
// int error = 0;
// socklen_t len = sizeof(error);
// int retval = getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &len);
// if (retval != 0)
// {
// /* there was a problem getting the error code */
// printf(stderr, "error getting socket error code: %s\n", strerror(retval));
// bad = -1;
// }
// if (error != 0)
// {
// /* socket has a non zero error status */
// printf(stderr, "socket error: %s\n", strerror(error));
// bad = -1;
// }
// printf("bad is %d\n",bad);
// return bad;
// }
/*
an alternative solution for checking if a socket is open - sending a magic string and
checking the return value of send()
this means that the client must support a magic string check (magic_recv())
int socket - the socket to check
returns -1 if the socket is closed, 0 otherwise
*/
int socket_is_open(int socket);
// to_string for client_s
void print_client(client_s *cl);
// print all the clients from the dynamic array
void print_all_clients();

/*
add a client to the dynamicly allocated array (clients)
int socket          - the client's socket
pthread_t thread id - the thread the client is running on
clientsPtr_s tmp    - a client struct to COPY the new client into, to avoid thread issues
doesn't return anything, but fills tmp with the new client
*/
void add_client(int socket, pthread_t threadID, clientPtr_s tmp);
/* remove the client_s with the specified socket from the dynamic array
int socket - the client socket to remove by
doesn't return anything
*/
void removeClient(int socket);
/*
pass an incoming message from sender to all the users save for the sender
msg_s recievedMessage - the message to pass
int sender - the client to ignore
doesn't return anything
*/
void pass_message(msg_s recviedMessage, int sender);
/* recv a message from a certian client
clientPtr_s cl - the client to listen to
returns -1 in case of an error, 1 otherwise
*/
int wait_for_msg(clientPtr_s cl);
void *clientHandler(void *vargp);
/*
call socket() bind() and listen() in this specific order using info
struct addrinfo *info - reference to server params
returns - the socket file descriptor on full success
*/
int socket_bind_listen(struct sockaddr_in *info);
/*
setting up addrinfo
struct addrinfo* serv - the sockaddr to setup
doesn't return anything
it doesn't work?????????????? getaddrinfo always returns -7 error
*/
void init_addrinfo(struct addrinfo *info);
/*
setting up sockaddr
struct sockaddr_in* serv - the sockaddr to setup
doesn't return anything

*/
void init_sockaddr(struct sockaddr_in *params);
void init_server();