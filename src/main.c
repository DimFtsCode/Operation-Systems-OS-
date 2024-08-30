#include "def.h"

SharedMemory* shared_memory = NULL; 




double exponential_random(double lambda) {
        double u = rand() / (double)RAND_MAX;
        return -log(1-u) / lambda;
}

void *request_handler(void *arg) {
    Request *request = (Request *) arg;
    // printf("\nHandler: start %d,stop %d,book %d , PTR %s",request->start,request->stop,request->book,request->sm_name);
    char book_name[6];
    sprintf(book_name, "%d.txt", request->book);
    //printf("\n %s ",book_name);
    FILE *file;
    char line[BLOCK]; 
    file = fopen(book_name, "r"); 
    if (file == NULL) {
        printf("File didnt open.\n");
        exit(1);
    }

    int fd = shm_open(request->sm_name, O_RDWR, 0666);
    Message* message =(Message*) mmap(0, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int value;
    sem_getvalue(&request->sem_server, &value);
    //printf("Price of request->sem_server: %d\n", value);

    int value1;
    sem_getvalue(&request->sem_client, &value1);
    //printf("Price of request->sem_client: %d\n", value1);

    int current_line = 1;
    while (fgets(line,sizeof(line), file) != NULL) {
        if (current_line >= request->start && current_line <= request->stop){
            //printf("LINE: %s",line);
            
            if (sem_wait(&request->sem_server) == -1) {
                perror("Σφάλμα στο sem_wait");
                
            }
            strcpy(message->line,line);
            //printf("Message line: %s",message->line);
            if (sem_post(&request->sem_client) == -1) {
                perror("Σφάλμα στο sem_post");
            }  
        }
        if (current_line == request->stop) {
            break; 
        }
        //printf("\n %d",current_line);
        current_line++;
        sem_destroy(&request->sem_client);
        sem_destroy(&request->sem_server);
    
    }
    fclose(file); 
    return NULL;
}


void file_server() {
    
    while(1) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        if (sem_timedwait(&shared_memory->request_available, &ts) == -1) {
            if (errno == ETIMEDOUT) {
                if (!shared_memory->file_server_flag) {
                    break;
                }
                continue;
            }
            else {
                perror("sem_timedwait");
                return;
            }
        }
        sem_wait(&shared_memory->semaphore);
        Request* request = receive_request();
        //printf("Request: start %d,stop %d,book %d , NAME %s",request->start,request->stop,request->book,request->name);
        sem_post(&shared_memory->semaphore);
        

        

        if(request != NULL) {
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, request_handler, request);
            pthread_detach(thread_id);
        }
        
    }
}

int main(int argc, char* argv[]) {
    if(argc < 4){ 
        printf("Wrong Inputs");
        return 1;
    }
    int N = atoi(argv[1]);
    int L = atoi(argv[2]);
    double lambda = atof(argv[3]);

    srand(time(NULL));
    char name[4];
    size_t size1 = sizeof(SharedMemory) ; // isws to kaneis global 
    strcpy(name, "sm1");
    shared_memory = (SharedMemory*) create_shared_memory(name, size1);
    pid_t pidFS = fork();
    if (pidFS == 0) {  
        file_server();
        exit(0);
    }
    //printf("Shared memory %p",shared_memory);
    for(int i=0; i<N; i++)
    {
        pid_t pid = fork();
        if (pid == 0) {
            srand(getpid());
            struct timespec start, end;

            if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
                perror("clock_gettime start error");
                return -1;
            }

            char local_name[16];
            sprintf(local_name, "local_shm_%d", i);
            //printf("local_name %s\n",local_name);
            Message* local_shared_memory = (Message*) create_local_memory(local_name, sizeof(Message));
            
            int lines = 0;
            int Books[10];
            for(int b=0; b<10; b++)
            {
                Books[b] = 0;
            }
            double total_time = 0.0;

            for (int j = 0; j < L; j++) {   
                int start = 1 + rand() % 9;
                int stop = start + 1 + rand() % (10 - start);
                int book = 1 + rand() % 10;
                Books[book-1]++;
                
                Request* request;
                request = send_request(start, stop, book, local_name);
                //sleep(1);
                
                int value;
                sem_getvalue(&request->sem_server, &value);
                //printf("Value of sem_server: %d\n", value);


                for(int i = start; i <= stop; i++) {
                   if (sem_wait(&request->sem_client) == -1) {
                        perror("Σφάλμα στο sem_client,sem_wait");
                    }
                    lines++;
                    //printf("eftasa for");

                    
                    //printf("local LINE:%s\n",local_shared_memory->line);
                    //sleep(1);
                    if (sem_post(&request->sem_server) == -1) {
                        perror("Σφάλμα στο sem_wait,sem_server");
                    }
                }
                double time = exponential_random(lambda);
                usleep(time);
                total_time = total_time + time;                
            }


            printf("Fork %d\n",i);
            printf("Total lines are: %d \t",lines);
            for(int b = 0; b<10; b++)
            {   
                if(Books[b]!=0){
                    printf("To vivlio: %d diavastike: %d \t",b+1,Books[b]);
                }
            }
            printf("\n");
            if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) {
                perror("clock_gettime end error");
                return -1;
            }
            double elapsed = (end.tv_sec - start.tv_sec) + 1e-9 * (end.tv_nsec - start.tv_nsec);
            
            printf("Elapsed time: %.9f seconds\n", elapsed);
            printf("Time between requests %.9f\n",total_time /L );
            free_local_memory(local_name, local_shared_memory);
            exit(0);
        }
    }
    for (int i = 0; i < N; i++) { 
        wait(NULL);
    }
    shared_memory->file_server_flag = 0;
    print_shared_memory();
    

    free_shared_memory(name);
    
    

    return 0;
}

