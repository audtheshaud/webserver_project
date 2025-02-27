#include <errno.h>      // Errno definitions
#include <arpa/inet.h>  // Internet type definitions
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
#include <pthread.h>    // Posix Threads
#include <semaphore.h>  // Semaphore definitions
#include <stdbool.h>    // Boolean definitions

struct args {
    int fd;                 // Client socket descriptor
    char server_msg[64];    // Server message
    int tid;                // Thread id
};

int create_server(int *fd, struct sockaddr_in *addr); /* Creates server socket descriptor and stores server details in an IPv4 socket address information struct */

char get_server_message();

int Socket(int *fd, int domain, int sock_type, int protocol);   /* Wrapper for socket() */

int Setsockopt(int fd, int level, int option_name, void *option_value, socklen_t option_length);

int Bind(int fd, struct sockaddr *addr, socklen_t length);

int Listen(int fd, int queued_clients);

int Accept(int fd, struct sockaddr *client_addr, socklen_t *client_addr_length);

int thread_search(int *tid);

int create_thread();

int build_thread_args(struct args *thread_args, int fd, void *message, int thread_id);