#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <stddef.h>
#include <time.h>


#define BLOCK 100
#define MAX_REQUESTS 1000


typedef struct {
    char line[BLOCK];
}Message;

typedef struct {
    int book;
    int start;
    int stop;
    char sm_name[16];
    sem_t sem_server;
    sem_t sem_client;
} Request;

typedef struct {
    sem_t semaphore;  
    sem_t request_available;  
    Request SharedMemorys[MAX_REQUESTS];
    int head;
    int tail;
    int file_server_flag;
} SharedMemory;


extern SharedMemory* shared_memory;


//basic_Share_memory
void* create_shared_memory(const char* name, size_t size);
Request* send_request(int start, int stop, int book, char* name);
void print_shared_memory();
void free_shared_memory(const char* name);
Request* receive_request(); 
//local_share_Memory
void* create_local_memory(const char* name, size_t size);
void free_local_memory(const char* name,Message* shared_str);
