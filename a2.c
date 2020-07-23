#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <semaphore.h>

int sem_id;

/// decrement by 1 the semaphore sem_no in the semaphore set sem_id
///    - ask for a permission, i.e. wait until a permission become available
void P(int sem_id, int sem_no){
    struct sembuf op = {sem_no, -1, 0};
    semop(sem_id, &op, 1);
}

/// increment by 1 the semaphore sem_no in the semaphore set sem_id
///   - release a previously obtained persmission
void V(int sem_id, int sem_no){
    struct sembuf op = {sem_no, +1, 0};
    semop(sem_id, &op, 1);
}

/// function executed by the 5 concurrent threads
/// maximum 5 threads are allowed to be simultaneously in their critical regions
/// this means that we cannot see more than 5 consecutive messages announcing the entrance, not followed by a leaving message
void* task3_func(void* arg){
    int th_id = *((int*) arg);
    P(sem_id, 2);
    if(th_id == 5){
        info(BEGIN, 6, th_id);
        V(sem_id, 1);
        P(sem_id, 0);
        info(END, 6, th_id);
    }
    else{
        if(th_id == 4){
            P(sem_id, 1);
            info(BEGIN, 6, th_id);
            info(END, 6, th_id);
            V(sem_id, 0);
        }
        else{
            if(th_id == 2){
                P(sem_id, 7);
                info(BEGIN, 6, 2);
                info(END, 6, 2);
                V(sem_id, 8);
            }
            else{
                info(BEGIN, 6, th_id);
                info(END, 6, th_id);
            }
        }
    }
    V(sem_id, 2);
    return NULL;
}

void task3(){

    /// Initialize the first and second semaphore in the set with 0 and third semaphore in the set with 5
    semctl(sem_id, 0, SETVAL, 0);
    semctl(sem_id, 1, SETVAL, 0);
    semctl(sem_id, 2, SETVAL, 5);

    /// create 5 competing threads
    int th_arg3[5];
    pthread_t t3[5];
    for(int i = 1; i <= 5; i++){
        th_arg3[i-1] = i;
        if(pthread_create(&t3[i-1], NULL, task3_func, &th_arg3[i-1])){
            perror("Cannot create a new thread!");
            exit(1);
            }
    }

    /// wait for the created threads to terminate
    for(int i = 1; i <= 5; i++){
        pthread_join(t3[i-1], NULL);
    }

    /// destroy (remove) the semaphore
    semctl(sem_id, 0, IPC_RMID, 0);
    semctl(sem_id, 1, IPC_RMID, 0);
    semctl(sem_id, 2, IPC_RMID, 0);
}

/// function executed by the 4 concurrent threads
/// maximum 4 threads are allowed to be simultaneously in their critical regions
/// this means that we cannot see more than 4 consecutive messages announcing the entrance, not followed by a leaving message
void* task4_func(void* arg){
    int th_id = *((int*) arg);
    P(sem_id, 3);
    if(th_id == 34) {
        info(BEGIN, 5, 34);
        V(sem_id, 4);
        P(sem_id, 5);
        info(END, 5, 34);
    }
    else{
        if(th_id == 35){
            info(BEGIN, 5, 35);
            V(sem_id, 4);
            P(sem_id, 5);
            info(END, 5, 35);
        }
        else{
            if(th_id == 36){
                info(BEGIN, 5, 36);
                V(sem_id, 4);
                P(sem_id, 5);
                info(END, 5, 36);
            }
            else{
                if(th_id == 14){
                    info(BEGIN, 5, 14);
                    P(sem_id, 4);
                    P(sem_id, 4);
                    P(sem_id, 4);
                    info(END, 5, 14);
                    V(sem_id, 5);
                    V(sem_id, 5);
                    V(sem_id, 5);
                }
                else{
                    info(BEGIN, 5, th_id);
                    info(END, 5, th_id);
                }
            }
        }
    }
    V(sem_id, 3);
    return NULL;
}

void task4(){
    semctl(sem_id, 3, SETVAL, 4);
    semctl(sem_id, 4, SETVAL, 0);
    semctl(sem_id, 5, SETVAL, 0);

    /// create 36 competing threads
    int th_arg4[36];
    pthread_t t4[36];
    for(int i = 1; i <= 36; i++){
        th_arg4[i-1] = i;
        if(pthread_create(&t4[i-1], NULL, task4_func, &th_arg4[i-1])){
            perror("Cannot create a new thread!");
            exit(1);
            }
    }

    /// wait for the created threads to terminate
    for(int i = 1; i <= 36; i++){
        pthread_join(t4[i-1], NULL);
    }

    /// destroy (remove) the semaphore
    semctl(sem_id, 3, IPC_RMID, 0);
    semctl(sem_id, 4, IPC_RMID, 0);
    semctl(sem_id, 5, IPC_RMID, 0);
}

void* task5_func(void* arg){
    int th_id = *((int*) arg);
    P(sem_id, 6);
    if(th_id == 4){
        info(BEGIN, 3, 4);
        V(sem_id, 7);
        info(END, 3, 4);
    }
    else{
        if(th_id == 3){
            P(sem_id, 8);
            info(BEGIN, 3, 3);
            info(END, 3, 3);
        }
        else{
            info(BEGIN, 3, th_id);
            info(END, 3, th_id);
        }
    }
    V(sem_id, 6);
    return NULL;
}

void task5(){

    /// Initialize the first and second semaphore in the set with 0 and third semaphore in the set with 5
    semctl(sem_id, 6, SETVAL, 5);
    semctl(sem_id, 7, SETVAL, 0);
    semctl(sem_id, 8, SETVAL, 0);

    /// create 5 competing threads
    int th_arg5[5];
    pthread_t t5[5];
    for(int i = 1; i <= 5; i++){
        th_arg5[i-1] = i;
        if(pthread_create(&t5[i-1], NULL, task5_func, &th_arg5[i-1])){
            perror("Cannot create a new thread!");
            exit(1);
            }
    }

    /// wait for the created threads to terminate
    for(int i = 1; i <= 5; i++){
        pthread_join(t5[i-1], NULL);
    }

    /// destroy (remove) the semaphore
    semctl(sem_id, 6, IPC_RMID, 0);
    semctl(sem_id, 7, IPC_RMID, 0);
    semctl(sem_id, 8, IPC_RMID, 0);
}

void createProcessHierarchy(){
    pid_t pid2 = fork();

    switch (pid2) {
        case -1: // error case
            perror("Cannot create a new child");
            exit(1);
        case 0: // child
            info(BEGIN, 2, 0);
            pid_t pid3 = fork();
            switch (pid3) {
                case -1: // error case
                    perror("Cannot create a new child");
                    exit(1);
                case 0: // child
                    info(BEGIN, 3, 0);
                    pid_t pid5 = fork();
                    switch (pid5) {
                        case -1: // error case
                            perror("Cannot create a new child");
                            exit(1);
                        case 0: // child
                            info(BEGIN, 5, 0);

                            task4();

                            info(END, 5, 0);
                            exit(0);
                        default: // parent
                            wait(NULL);
                    }

                    task5();

                    info(END, 3, 0);
                    exit(0);
                default: // parent
                    wait(NULL);
            }
            info(END, 2, 0);
            exit(0);
        default: // parent
            wait(NULL);
    }

    pid_t pid4 = fork();

    switch (pid4) {
        case -1: //error case
            perror("Cannot create a new child");
            exit(1);
        case 0: //child
            info(BEGIN, 4, 0);
            info(END, 4, 0);
            exit(0);
        default: //parent
            wait(NULL);
    }

    pid_t pid6 = fork();

    switch(pid6){
        case -1: //error case
            perror("Cannot create a new child");
            exit(1);
        case 0: //child
            info(BEGIN, 6, 0);
            pid_t pid7 = fork();
            switch (pid7) {
                case -1: // error case
                    perror("Cannot create a new child");
                    exit(1);
                case 0: // child
                    info(BEGIN, 7, 0);
                    info(END, 7, 0);
                    exit(0);
                default: // parent
                    wait(NULL);
            }

            task3();

            info(END, 6, 0);
            exit(0);
        default: //parent
            wait(NULL);
    }
}

int main(){
    init();

    info(BEGIN, 1, 0);

    sem_id = semget(IPC_PRIVATE, 9, IPC_CREAT | 0600);
        if (sem_id < 0) {
            perror("Error creating the semaphore set");
            exit(2);
        }
    createProcessHierarchy();

    info(END, 1, 0);
    return 0;

}
