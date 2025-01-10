# Concurrent Multi-threaded Server and Client
Using C, I designed a concurrent server that creates a thread for each client connection. I use an array of five pthread_t variables to accomplish this. As each client is accepted, I call first dynamically allocate the thread argument struct using malloc and then call pthread_create. To join the thread I use pthread_detach instead of pthread_join so that each thread becomes independent of each other to prevent threads from terminating another thread's client connection.

This project was inspired by the assignment document I found online. I used it as a starting point to design a simple client and server.
