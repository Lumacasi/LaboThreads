#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

struct sigaction action;
sigset_t mask;

void *fctThreadEtape2(void *param);

void *fctMasterThreadEtape2(void *param);

void handler(int sig);

void thread_handler(int sig);

pthread_t threadHandle1, threadHandle2, threadHandle3, threadHandle4, threadHandle5;

int main(){

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    int ret, *retThread1, *retThread2, *retThread3, *retThread4;

    ret = pthread_create(&threadHandle1, NULL, (void *(*) (void *))fctThreadEtape2, NULL);
    ret = pthread_create(&threadHandle2, NULL, (void *(*) (void *))fctThreadEtape2, NULL);
    ret = pthread_create(&threadHandle3, NULL, (void *(*) (void *))fctThreadEtape2, NULL);
    ret = pthread_create(&threadHandle4, NULL, (void *(*) (void *))fctThreadEtape2, NULL);
    ret = pthread_create(&threadHandle5, NULL, (void *(*) (void *))fctMasterThreadEtape2, NULL);

    pause();

    pthread_exit(0);
}

void *fctThreadEtape2(void * param){
    struct sigaction thread_action;
    thread_action.sa_handler = thread_handler;
    sigemptyset(&thread_action.sa_mask);
    thread_action.sa_flags = 0;
    sigaction(SIGUSR1, &thread_action, NULL);

    printf("thread : %p, en attente de message !\n", pthread_self());

    pause();

    printf("thread : %p, message reçu !\n", pthread_self());

    pthread_exit(0);
}

void *fctMasterThreadEtape2(void *param){

    printf("master thread : %p, en attente de message\n", pthread_self());

    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    while(1){
        pause();
        break;
    }

    printf("master thread : %p, message reçu !\n", pthread_self());

    pthread_exit(0);
}

void handler(int sig){
    printf("\n");
    pthread_kill(threadHandle1, SIGUSR1);
    pthread_kill(threadHandle2, SIGUSR1);
    pthread_kill(threadHandle3, SIGUSR1);
    pthread_kill(threadHandle4, SIGUSR1);
}

void thread_handler(int sig){}