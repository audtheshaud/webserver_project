#include <errno.h>      // Errno definitions
#include <stdio.h>      // Standard Input/Output definitions
#include <stdlib.h>     // Standard library function definitions
#include <unistd.h>     // POSIX Operating System API definitions
#include <sys/types.h>  // Data types definitions
#include <sys/socket.h> // Sockets definitions
#include <arpa/inet.h>  // Internet definitions
#include <netinet/in.h> // Internet Address Family definitions
#include <netdb.h>      // Address definitions
#include <limits.h>     // Limit definitions
#include <string.h>     // String definitions
#include <stdbool.h>    // Boolean definitions

int create_client(int *fd, struct sockaddr_in *addr);           /* Creates and stores client details in a IPv4 socket address information struct */


int Socket(int *fd, int domain, int sock_type, int protocol);   /* Wrapper for socket() */

int Connect(int fd, struct sockaddr *addr, socklen_t length);   /* Wrapper for connect() */

int Recv(int fd, void *buffer, size_t buf_size, int flags);     /* Wrapper for recv() */

int Send(int fd, void *buffer, size_t buf_size, int flags);     /* Wrapper for Socket() */

int recieve_server_message(int fd);                             /* Recieves and prints the server's message in the client's terminal */

int send_chat_message(int fd);                                  /* Sends the client's messages to the server's terminal */