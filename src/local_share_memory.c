#include "def.h"

void* create_local_memory(const char* name, size_t size) {
    
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

    Message* shared_str = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_str == MAP_FAILED) {
        perror("mmap");
        exit(1);
    
    }
    
    close(shm_fd);
    return shared_str;
}


void free_local_memory(const char* name,Message* shared_str) {
    if(shared_memory) {
        
        munmap(shared_str, sizeof(Message));
       
        shm_unlink(name);

        
    }
}