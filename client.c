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

#define PORT 5555

int main() {
    int client_descriptor; // Socket file descriptor
    struct sockaddr_in client_addr; // Server socket address information for IPv4 addresses

    if ((client_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){ // Create socket that uses the IPv4 address family, Reliable Byte-stream, and TCP
        printf("Failed to create TCP socket: %s\n", strerror(errno)); // If creating the socket fails then exit the program
        exit(0);
    }

    client_addr.sin_family = AF_INET; // Set address family to IPv4
    client_addr.sin_port = htons(PORT); // Set port using Host to Network Short
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Allow any IP addresses available to the Host to connect

    int connect_status;
    if ((connect_status=connect(client_descriptor, (struct sockaddr *)&client_addr, sizeof(client_addr))) < 0){
        printf("Failed to connect to server: %s\n", strerror(errno)); // If connecting to the server fails then exit the program
        exit(0);
    } else {
        char server_message[64];
        memset(&server_message, 0, sizeof(server_message));
        if ((recv(client_descriptor, server_message, sizeof(server_message), 0)) < 0){
            printf("Failed to receive to server: %s\n", strerror(errno)); // If receiving the server's message fails then exit the program
            exit(0);
        }
        printf("Recived: %s\n", server_message);
    }
    close(client_descriptor);
    return 0;
}