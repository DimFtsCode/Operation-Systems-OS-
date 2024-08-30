#include "def.h"
//Working as a cycle creating a list of requests inside share_memory
Request* send_request(int start, int stop, int book, char* name) {
    sem_wait(&shared_memory->semaphore);

    if ((shared_memory->head + 1) % MAX_REQUESTS == shared_memory->tail) {
        fprintf(stderr, "Shared memory is full!\n");
        sem_post(&shared_memory->semaphore); 
        return NULL;
    }


    shared_memory->SharedMemorys[shared_memory->head].start = start;
    shared_memory->SharedMemorys[shared_memory->head].stop = stop;
    shared_memory->SharedMemorys[shared_memory->head].book = book;
    strcpy(shared_memory->SharedMemorys[shared_memory->head].sm_name ,name);
    sem_init(&shared_memory->SharedMemorys[shared_memory->head].sem_server,1,1); 
    sem_init(&shared_memory->SharedMemorys[shared_memory->head].sem_client,1,0); 
    //printf("Inside send: %s",shared_memory->SharedMemorys[shared_memory->head].sm_name);
    int k = shared_memory->head;
    shared_memory->head = (shared_memory->head + 1) % MAX_REQUESTS;
    sem_post(&shared_memory->request_available);
    sem_post(&shared_memory->semaphore);
    return &shared_memory->SharedMemorys[k];
}
//Returning the request 
Request* receive_request() {
    if (shared_memory->tail == shared_memory->head) {
        // Ο buffer is empty
        return NULL;
    }
    
    shared_memory->tail = (shared_memory->tail + 1) % MAX_REQUESTS;
    return &shared_memory->SharedMemorys[shared_memory->tail-1];
}

void* create_shared_memory(const char* name, size_t size) {
    
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return NULL;
    }

    
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return NULL;
    }

    
    SharedMemory *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        return NULL;
    }
    sem_init(&memory->semaphore, 1, 1);
    sem_init(&memory->request_available, 1, 0);

    
    memory->head = 0;
    memory->tail = 0;
    memory->file_server_flag = 1;

    close(shm_fd);
    return memory;
}



void free_shared_memory(const char* name) {
    if(shared_memory) {

        sem_destroy(&shared_memory->semaphore);
        sem_destroy(&shared_memory->request_available);
        munmap(shared_memory, sizeof(SharedMemory));
        shm_unlink(name);
        
    }
}


void print_shared_memory() {
    printf("Shared Memory Contents:\n");
    printf("-------------------------\n");
    printf("Head: %d\n", shared_memory->head);
    printf("Tail: %d\n", shared_memory->tail);
    printf("\nRequests:\n");

    for (int i = 0; i < MAX_REQUESTS; i++) {
        if (shared_memory->SharedMemorys[i].book != 0) {
            printf("Position %d: %d\n", i, shared_memory->SharedMemorys[i].book);
        }
    }

    printf("-------------------------\n");
}