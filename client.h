#include "chat.h"
int exit_on_error(int status, char *str);
/*
basiaclly a recv() extention - the function loops recieve until it recieves anything but
the magic string, and stores it into buffer.
int socket - the socket to call recv() on
msg_s *buffer - the content recieved
the function returns the recv() content by saving it into the buffer
*/
int magic_recv(int socket, msg_s *buffer);
/*
The thread that is getting messages from the server
void *socket - the servers socket
doesn't return anything
*/
void *server_listener(void *socket);
/*
set up sockaddr
sockaddr_in *params - the sockaddr to fill
doesn't return anything
*/
void init_sockaddr(struct sockaddr_in *params);
// func to call from main
void init_client();