#include "server.h"

#define PORT 5555        // Arbitrary port
#define MAXTHREADCOUNT 5 // Maximum thread count

/*
Notes: use a mutex lock on an array of 0s and 1s (1s being busy) to keep track of which thread is being utilized
and then use the semaphore to know that a thread is no longer being used when it decrements from 5
*/

int busy_threads[MAXTHREADCOUNT] = {0}; // Creates array of size MAXTHREADCOUNT for tracking busy threads, intitalized to 0
sem_t thread_lock;                                 // Locks threads, value is from 0 to 5
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for accessing array of busy threads


void *handle_client(void *args)
{ // Thread that handles each client by sending a server message and closing the connection
    struct args *arguments = args; // Set struct of type struct void* to struct* args
    char tid_msg[32];
    snprintf(tid_msg, sizeof(tid_msg), ", Connected to thread id: %d", arguments->tid);
    strcat(arguments->server_msg, tid_msg);
    if ((send(arguments->fd, arguments->server_msg, strlen(arguments->server_msg), 0)) < 0)
    {                                                              // Send the Server message using the Client's socket descriptor
        printf("Failed to send on socket: %s\n", strerror(errno)); // If sending fails then exit the program
        exit(0);
    }
    char client_message[64];
    bool connection = true;
    while(connection){
        if ((recv(arguments->fd, client_message, sizeof(client_message), 0)) < 0){
            printf("Failed to receive to server: %s\n", strerror(errno)); // If receiving the server's message fails then exit the program
            exit(0);
        }
        if (strcmp(client_message, "logout") != 0 || strcmp(client_message, "Logout") != 0){
            connection = false;
        }
    }
    printf("Client on Thread ID: %d disconnected\n", arguments->tid);
    // Client needs to send a "disconnect" message to the server
    shutdown(arguments->fd, SHUT_RDWR); // Shutdown the Client's socket descriptor to prevent reading and writing
    close(arguments->fd);               // Close the Client's socket descriptor
    free(arguments);                           // Free the arguments struct, requires the pointer to the struct
    pthread_mutex_lock(&mutex);                // Lock the thread to change the value of busy threads
    busy_threads[arguments->tid] = 0;          // Thread to not busy by setting it to 0
    pthread_mutex_unlock(&mutex);              // Unlock the thread
    sem_post(&thread_lock);                    // Mark the thread as being free
    return NULL; // Return from the thread
}


int main()
{
    int server_fd;                      // Socket file descriptor
    struct sockaddr_in server_addr;     // Server socket address information for IPv4 addresses
    
    char server_message = get_server_message();

    create_server(&server_fd, &server_addr);

    if ((bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {                                                           // Bind the socket address (IPv4 + Port)
        printf("Failed to bind socket: %s\n", strerror(errno)); // If binding the socket fails then exit the program
        exit(0);
    }

    if ((listen(server_fd, 10)) < 0)
    {                                                                // Listen for one client using Socket Descriptor
        printf("Failed to listen on socket: %s\n", strerror(errno)); // If listening fails then exit the program
        exit(0);
    }
    else
    {
        printf("Listening...\n");
    }
    int client_descriptor;                         // Create a Client socket descriptor
    struct sockaddr_in client_addr;                // Create a socket address IPv4 struct for the Client
    memset(&client_addr, 0, sizeof(client_addr));  // Initialize the Client socket address IPv4 struct to 0
    socklen_t client_length = sizeof(client_addr); // Get the length of the Client socket address IPv4 struct

    pthread_t threads[MAXTHREADCOUNT]; // Create a pool of threads
    struct args thread_args;

    sem_init(&thread_lock, 0, MAXTHREADCOUNT); // Creates a semphore with a value of 5

    int lock_value;
    int tid;
    while (1)
    {
        sem_getvalue(&thread_lock, &lock_value);
        if ( lock_value > 0){ // Check if there is a free thread using the Thread Lock Semaphore
            pthread_mutex_lock(&mutex);
            for (tid = 0; tid < 4; tid++) // If a thread is free, find the Thread ID 
            {   
                printf("Index: %d %d\n", tid, busy_threads[tid]);
                if (busy_threads[tid] == 0)
                {
                    break;
                }
            }
            printf("Thread ID: %d was found\n", tid);   // Found Thread ID 
            busy_threads[tid] = 1;                      // Set Thread ID to busy by setting it to 1
            sem_wait(&thread_lock);                     // Mark the thread as being busy
            pthread_mutex_unlock(&mutex);               // Unlock the thread
            if ((client_descriptor = accept(server_fd, (struct sockaddr *)&client_addr, &client_length)) < 0)
            {
                printf("Failed to accept client on socket: %s\n", strerror(errno)); // If accepting fails then exit the program
                exit(0);
            }
            char client_ip[INET_ADDRSTRLEN];                                       // Create a buffer to get the Client's IP Address
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // IPv4 Network to Presentation
            printf("Successfully connected to client: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("--------------------------------\n");

            struct args *thread_args = malloc(sizeof(struct args));              // Dynamically allocate a Thread Argument Struct
            thread_args->fd = client_descriptor;                          // Save the Client Socket Descriptor to the Struct
            memset(thread_args->server_msg, 0, sizeof(thread_args->server_msg)); // Initialize the Server message to 0
            strcpy(thread_args->server_msg, server_message);                     // Save the server message to the Struct
            thread_args->tid = tid;                                            // Set the TID to the index of the for loop

            if ((pthread_create(&threads[tid], NULL, handle_client, (void *)thread_args)) != 0) // Create the thread
            {
                printf("Failed to create thread for client: %s\n", strerror(errno));
                exit(0);
            }
            if (pthread_detach(threads[tid]) != 0) // Detach the thread to prevent Clients from terminating other Clients
            {
                printf("Failed to detach thread for client: %s\n", strerror(errno));
                exit(0);
            }
        } else {
            printf("All Threads are busy, please wait for a client to disconnect...\n");
            sleep(3);
        }
    }

    printf("All threads have been utilized, powering off the server...\n");
    sleep(5);
    close(server_fd);
    pthread_exit(NULL);
    return 0;
}

char get_server_message(){
    int user_input;
    printf("Enter any number from 0-100: ");                                            // Prompt user for random number between 1-100
    while ((scanf("%3d", &user_input) != 1) || user_input < 1 || user_input > 100){     // Limiting scanf input to max of 3 characters, while checking num is between 1 and 100
        printf("Please re-enter a number between 0-100...\n");                          // Re-prompt the user if the input does not meet conditionals
    }
    char id[4];                                     // Instantiate a buffer for the user_input to be converted to a string
    snprintf(id, sizeof(id), "%d", user_input);     // Conversion of int to char
    char message[32] = "Aud's Server: #";           // Instantiate the string containing the server's name and id
    strcat(message, id);                            // Concatenate the server name and the id
    return message;                                 // Return the server message
}

int create_server(int *fd, struct sockaddr_in *addr){
    Socket(fd, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    Setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    addr->sin_family = AF_INET;                         // Set address family to IPv4
    addr->sin_port = htons(PORT);                       // Set port using Host to Network Short
    addr->sin_addr.s_addr = inet_addr("127.0.0.1");     // Allow any IP addresses available to the Host to connect
    Bind(*fd, addr, sizeof(*addr));                     // Bind IP address and port to create socket address
    Listen(*fd, 10);                                    // Listen on socket, allow 10 clients to queue up
}

int Socket(int *fd, int domain, int sock_type, int protocol){           // Wrapper for socket function call
    if ((*fd = socket(domain, sock_type, protocol)) < 0){               // Create socket that uses the IPv4 address family, Reliable Byte-stream, and TCP
        printf("Failed to create TCP socket: %s\n", strerror(errno));   // If creating the socket fails then exit the program
        exit(EXIT_FAILURE);
    }
    return 0;
}

int Setsockopt(int fd, int level, int option_name, void *option_value, socklen_t option_length){
    if (setsockopt(fd, level, option_name, option_value, option_length) < 0){
        printf("Failed to set socket options: %s\n", strerror(errno));   // If setting the socket option fails then exit the program
        exit(EXIT_FAILURE);
    }
    return 0;
}

int Bind(int fd, struct sockaddr *addr, socklen_t length){
    if (bind(fd, addr, length) < 0){
        printf("Failed to bind socket: %s\n", strerror(errno));     // If binding the socket fails then exit the program
        exit(0);
    }
    return 0;
}

int Listen(int fd, int queued_clients){
    if (listen(fd, queued_clients) < 0){
        printf("Failed to listen on socket: %s\n", strerror(errno)); // If listening fails then exit the program
        exit(0);
    }
    printf("Listening...\n");
    return 0;
}








/*
while (client_count < MAXTHREADCOUNT+1)
{ // Thread initilization loop, creates and detaches threads for each Client connection up to the maximum thread count
    if ((client_descriptor = accept(server_descriptor, (struct sockaddr *)&client_addr, &client_length)) < 0)
    {
        printf("Failed to accept client on socket: %s\n", strerror(errno)); // If accepting fails then exit the program
        exit(0);
    }
    char client_ip[INET_ADDRSTRLEN];                                                             // Create a buffer to get the Client's IP Address
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);                       // IPv4 Network to Presentation
    printf("Successfully connected to client: %s:%d\n", client_ip, ntohs(client_addr.sin_port)); // Print confirmtion of the Client's connection
    client_count += 1;                                                                           // Increase the Client count
    printf("Clients connected: %d\n\n", client_count + 1);                                       // Print the amount of connected clients

    struct args *thread_args = malloc(sizeof(struct args));              // Dynamically allocate a Thread Argument Struct
    thread_args->client_sd = client_descriptor;                          // Save the Client Socket Descriptor to the Struct
    memset(thread_args->server_msg, 0, sizeof(thread_args->server_msg)); // Initialize the Server message to 0
    strcpy(thread_args->server_msg, server_message);                     // Save the server message to the Struct
    thread_args->tid = client_count;

    if ((pthread_create(&threads[client_count], NULL, handle_client, (void *)thread_args)) != 0) // Create the thread
    {
        printf("Failed to create thread for client: %s\n", strerror(errno));
        exit(0);
    }
    if (pthread_detach(threads[client_count]) != 0) // Detach the thread to prevent Clients from terminating other Clients
    {
        printf("Failed to detach thread for client: %s\n", strerror(errno));
        exit(0);
    }
}
printf("All threads have been utilized, beginning second loop...");
sleep(3);
*/

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

    IMPORTANT CODE BELOW: 

pthread_mutex_lock(&mutex);
        for (index = 0; index <= 5; index++)
        {   
            printf("Index: %d %d\n", index, busy_threads[index]);
            if (busy_threads[index] == 0)
            {
                break;
            }
        }
        if (index > MAXTHREADCOUNT)
        {
            printf("All Threads are busy, please wait for a client to disconnect...\n");
            sleep(3);
            pthread_mutex_unlock(&mutex);
            continue;
        }
        else
        {
            printf("Thread with tid: %d was found\n", index);
            busy_threads[index] = 1;      // Thread to busy by setting it to 1
            pthread_mutex_unlock(&mutex); // Unlock the thread
            if ((client_descriptor = accept(server_descriptor, (struct sockaddr *)&client_addr, &client_length)) < 0)
            {
                printf("Failed to accept client on socket: %s\n", strerror(errno)); // If accepting fails then exit the program
                exit(0);
            }
            char client_ip[INET_ADDRSTRLEN];                                       // Create a buffer to get the Client's IP Address
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // IPv4 Network to Presentation
            printf("Successfully connected to client: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("--------------------------------\n");

            struct args *thread_args = malloc(sizeof(struct args));              // Dynamically allocate a Thread Argument Struct
            thread_args->client_sd = client_descriptor;                          // Save the Client Socket Descriptor to the Struct
            memset(thread_args->server_msg, 0, sizeof(thread_args->server_msg)); // Initialize the Server message to 0
            strcpy(thread_args->server_msg, server_message);                     // Save the server message to the Struct
            thread_args->tid = index;                                            // Set the TID to the index of the for loop

            if ((pthread_create(&threads[index], NULL, handle_client, (void *)thread_args)) != 0) // Create the thread
            {
                printf("Failed to create thread for client: %s\n", strerror(errno));
                exit(0);
            }
            if (pthread_detach(threads[index]) != 0) // Detach the thread to prevent Clients from terminating other Clients
            {
                printf("Failed to detach thread for client: %s\n", strerror(errno));
                exit(0);
            }
        }



void *handle_client(void *args)
{ // Thread that handles each client by sending a server message and closing the connection
    struct args *arguments = args; // Set struct of type struct void* to struct* args
    char tid_msg[32];
    snprintf(tid_msg, sizeof(tid_msg), ", Connected to thread id: %d", arguments->tid);
    strcat(arguments->server_msg, tid_msg);
    if ((send(arguments->client_sd, arguments->server_msg, strlen(arguments->server_msg), 0)) < 0)
    {                                                              // Send the Server message using the Client's socket descriptor
        printf("Failed to send on socket: %s\n", strerror(errno)); // If sending fails then exit the program
        exit(0);
    }

    sleep(10);
    // Client needs to send a "disconnect" message to the server
    shutdown(arguments->client_sd, SHUT_RDWR); // Shutdown the Client's socket descriptor to prevent reading and writing
    close(arguments->client_sd);               // Close the Client's socket descriptor
    free(arguments);                           // Free the arguments struct, requires the pointer to the struct
    pthread_mutex_lock(&mutex);                // Lock the thread to change the value of busy threads
    busy_threads[arguments->tid] = 0;          // Thread to not busy by setting it to 0
    pthread_mutex_unlock(&mutex);              // Unlock the thread
    sem_post(&thread_lock);                    // Mark the thread as being free
    return NULL; // Return from the thread
}

struct args
{                        // Struct of args for each thread
    int client_sd;       // Client socket descriptor
    char server_msg[64]; // Server message
    int tid;             // Thread id
};


int server_id;
    printf("Enter a number between 1-100: "); // Prompt user for random number between 1-100
    while ((scanf("%3d", &server_id) != 1) || server_id < 1 || server_id > 100)
    { // Limiting scanf input to max of 3 characters while checking num is between 1 and 100
        printf("Please enter a number between 1-100...\n");
    }

    char str[4];
    snprintf(str, sizeof(str), "%d", server_id); // Conversion of int to char
    char server_name[] = "Aud's Server: #";
    char server_message[32];
    strcpy(server_message, server_name); // Copy the server name into the message
    strcat(server_message, str);         // Concatenate the server name and the number
    printf("Server message: %s\n", server_message);

    

    if ((server_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {                                                                 // Create socket that uses the IPv4 address family, Reliable Byte-stream, and TCP
        printf("Failed to create TCP socket: %s\n", strerror(errno)); // If creating the socket fails then exit the program
        exit(0);
    }

    int opt = 1;
    setsockopt(server_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;                     // Set address family to IPv4
    server_addr.sin_port = htons(PORT);                   // Set port using Host to Network Short
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Allow any IP addresses available to the Host to connect

*/