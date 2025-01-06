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

#define PORT 5555
#define THREADCOUNT 5

struct args{
    int client_sd;
    char server_msg[128];
    int tid;
};

void* handle_client(void* args){
    struct args* arguments = args;
    if ((send(arguments->client_sd, arguments->server_msg, strlen(arguments->server_msg),0)) < 0){
        printf("Failed to send on socket: %s\n", strerror(errno)); // If sending fails then exit the program
        exit(0);
    }
    shutdown(arguments->client_sd, SHUT_RDWR);
    close(arguments->client_sd);
    free(arguments);
    return NULL;
}

int main() {
    int num;
    printf("Enter a number between 1-100: "); // Prompt user for random number between 1-100
    while ((scanf("%3d", &num) != 1) || num < 1 ||  num > 100){ // Limiting scanf input to max of 3 characters while checking num is between 1 and 100
        printf("Please enter a number between 1-100...\n");
    } 
    printf("Your number is %d\n", num); // Print the user's chosen number

    char str[4];
    snprintf(str, sizeof(str), "%d", num); // Conversion of int to char
    char server_name[] = "Aud's Server: #";
    char server_message[128];
    strcpy(server_message, server_name); // Copy the server name into the message
    strcat(server_message, str); // Concatenate the server name and the number
    printf("Server message: %s\n", server_message);

    int server_descriptor; // Socket file descriptor
    struct sockaddr_in server_addr; // Server socket address information for IPv4 addresses

    if ((server_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){ // Create socket that uses the IPv4 address family, Reliable Byte-stream, and TCP
        printf("Failed to create TCP socket: %s\n", strerror(errno)); // If creating the socket fails then exit the program
        exit(0);
    }

    int opt = 1;
    setsockopt(server_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET; // Set address family to IPv4
    server_addr.sin_port = htons(PORT); // Set port using Host to Network Short
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Allow any IP addresses available to the Host to connect
    
    if ((bind(server_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0){ // Bind the socket address (IPv4 + Port)
        printf("Failed to bind socket: %s\n", strerror(errno)); // If binding the socket fails then exit the program
        exit(0);
    }
    
    if ((listen(server_descriptor, 10)) < 0){ // Listen for one client using Socket Descriptor
        printf("Failed to listen on socket: %s\n", strerror(errno)); // If listening fails then exit the program
        exit(0);
    } else{
        printf("Listening...\n");
    }
    int client_descriptor;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_length = sizeof(client_addr);
    
    int client_count = -1;
    pthread_t threads[THREADCOUNT];
    struct args thread_args;

    while(1){
        if (client_count == THREADCOUNT){
            //printf("Max number of clients has been reached, please wait for another client to disconnect...\n");
            break;
        }else{
            if ((client_descriptor = accept(server_descriptor, (struct sockaddr *)&client_addr, &client_length)) < 0){
            printf("Failed to accept on socket: %s\n", strerror(errno)); // If accepting fails then exit the program
            exit(0);
            }
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // IPv4 Network to Presentation
            printf("Successfully connected to client: %s:%d\n", client_ip,ntohs(client_addr.sin_port));
            client_count += 1;
            printf("Clients connected: %d\n\n", client_count+1);

            struct args* thread_args = malloc(sizeof(struct args));
            thread_args->client_sd = client_descriptor;
            memset(thread_args->server_msg, 0, sizeof(thread_args->server_msg));
            strcpy(thread_args->server_msg, server_message);
            thread_args->tid = client_count;

            if ((pthread_create(&threads[client_count], NULL, handle_client, (void *)thread_args)) != 0){
                printf("Failed to create thread for client: %s\n", strerror(errno));
                exit(0);
            }
            if (pthread_detach(threads[client_count]) != 0){
                printf("Failed to detach thread for client: %s\n", strerror(errno));
                exit(0);
            }
        }

    }
    printf("All threads have been utilized, powering off the server...\n");
    sleep(5);
    close(server_descriptor);
    pthread_exit(NULL);
    return 0;
}


/*
char thread_id[2];
snprintf(thread_id, sizeof(thread_id), "%d", arguments->tid);

int client_descriptor;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));

    socklen_t client_length = sizeof(client_addr);
    if ((client_descriptor = accept(server_descriptor, (struct sockaddr *)&client_addr, &client_length)) < 0){
        printf("Failed to accept on socket: %s\n", strerror(errno)); // If accepting fails then exit the program
        exit(0);
    }
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // Ipv4 Network to Presentation
    printf("Successfully connected to client: %s:%d\n", client_ip,ntohs(client_addr.sin_port));

    if ((send(client_descriptor, server_message, sizeof(server_message),0)) < 0){
        printf("Failed to send on socket: %s\n", strerror(errno)); // If sending fails then exit the program
        exit(0);
    }

*/