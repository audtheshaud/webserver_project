#include "client.h"

#define PORT 5555

int main() {
    int client_descriptor; // Socket file descriptor
    struct sockaddr_in server_addr_info; // Server socket address information for IPv4 addresses

    create_client(&client_descriptor,&server_addr_info);

    Connect(client_descriptor, (struct sockaddr *)&server_addr_info, sizeof(server_addr_info));

    char server_message[64];

    recieve_server_message(client_descriptor);

    send_chat_message(client_descriptor);
    
    close(client_descriptor);
    return 0;
}

int create_client(int *fd, struct sockaddr_in *addr){
    Socket(fd, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr->sin_family = AF_INET;                         // Set address family to IPv4
    addr->sin_port = htons(PORT);                       // Set port using Host to Network Short
    addr->sin_addr.s_addr = htonl(INADDR_ANY);          // Allow any IP addresses available to the Host to connect
    return 0;
}

int Socket(int *fd, int domain, int sock_type, int protocol){           // Wrapper for socket function call
    if ((*fd = socket(domain, sock_type, protocol)) < 0){               // Create socket that uses the IPv4 address family, Reliable Byte-stream, and TCP
        printf("Failed to create TCP socket: %s\n", strerror(errno));   // If creating the socket fails then exit the program
        exit(EXIT_FAILURE);
    }
    return 0;
}

int Connect(int fd, struct sockaddr *addr, socklen_t length){
    if (connect(fd, addr, length) < 0){
        printf("Failed to connect to server: %s\n", strerror(errno));   // If connecting to the server fails then exit the program
        exit(0);
    }
    printf("Connected to Server\n");                                    // On success, print out connection message to the client's terminal
    printf("--------------------------------\n");                       // Border for readability
    return 0;
}

int recieve_server_message(int fd){
    char server_message[64];                                            // Server message buffer
    Recv(fd, server_message, strlen(server_message), 0);                // Call recv wrapper
    printf("Recived: %s\n", server_message);                            // Print out the server message to the client's terminal
    return 0;
}

int Recv(int fd, void * buffer, size_t buf_size, int flags){
    if (recv(fd, buffer,buf_size, flags) < 0){
        printf("Failed to receive to server: %s\n", strerror(errno));   // If receiving the server's message fails then exit the program
        exit(EXIT_FAILURE);
    }
}

int send_chat_message(int fd){
    char client_message[64];                                            // Create client message buffer
    bool connection = true;                                             // Set connection to true before entering messaging loop
    while (connection){                                                 // Prompts user to enter message to server until selecting to logout
        printf("Enter a message to the server: ");
        scanf("%63s", client_message);
        Send(fd, client_message, strlen(client_message), 0);
        if (strcmp(client_message, "logout") == 0 || strcmp(client_message, "Logout") == 0){
            connection = false;
        }
    }
}

int Send(int fd, void *buffer, size_t buf_size, int flags){
    if ((send(fd, buffer, strlen(buffer), 0)) < 0){                     // Send the Client message using the Client's socket descriptor
        printf("Failed to send on socket: %s\n", strerror(errno));      // If sending fails then exit the program
        exit(0);
    }
}