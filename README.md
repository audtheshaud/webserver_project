# Concurrent Multi-threaded Server and Client
Using C, I designed a concurrent server that creates a thread for each client connection. I use an array of five pthread_t variables to accomplish this. As each client is accepted, I first dynamically allocate the thread argument struct using malloc and then call pthread_create. To start the thread I use pthread_detach instead of pthread_join so that each thread can be reused after the client disconnects. As of now, the threads can properly cycle between being marked "busy" and "not busy" using a semaphore to check the amount of busy threads and a mutex to edit a list of thread statuses.

This project was inspired by the assignment document I found online. I used it as a starting point to design a simple client and server.
