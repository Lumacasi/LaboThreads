#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


void *fctThreadEtape4(void *param);

void *fctMasterThreadEtape4(void *param);
void fctMasterThreadEtape4Fin(void *p);

void handler(int sig);

void thread_handler(int sig);

pthread_t threadHandle1, threadHandle2, threadHandle3, threadHandle4, threadHandle5;

int main(){

    struct sigaction action;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    int ret, *retThread1, *retThread2, *retThread3, *retThread4, *retThread5;

    ret = pthread_create(&threadHandle1, NULL, (void *(*) (void *))fctMasterThreadEtape4, NULL);
    ret = pthread_create(&threadHandle2, NULL, (void *(*) (void *))fctThreadEtape4, NULL);
    ret = pthread_create(&threadHandle3, NULL, (void *(*) (void *))fctThreadEtape4, NULL);
    ret = pthread_create(&threadHandle4, NULL, (void *(*) (void *))fctThreadEtape4, NULL);
    ret = pthread_create(&threadHandle5, NULL, (void *(*) (void *))fctThreadEtape4, NULL);

    pthread_join(threadHandle2, (void **)&retThread2);
    pthread_join(threadHandle3, (void **)&retThread3);
    pthread_join(threadHandle4, (void **)&retThread4);
    pthread_join(threadHandle5, (void **)&retThread5);

    pthread_cancel(threadHandle1);
    pthread_join(threadHandle1, (void **)&retThread1);

    exit(0);
}

void *fctThreadEtape4(void * param){
    struct sigaction action;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    struct sigaction thread_action;
    thread_action.sa_handler = thread_handler;
    sigemptyset(&thread_action.sa_mask);
    thread_action.sa_flags = 0;
    sigaction(SIGUSR1, &thread_action, NULL);

    printf("thread : %p, en attente de message !\n", pthread_self());

    pause();

    pthread_exit(0);
}

void *fctMasterThreadEtape4(void *param){

    struct sigaction action;
    sigset_t mask;
    int etat;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    
    pthread_cleanup_push(fctMasterThreadEtape4Fin, NULL);
    printf("master thread : %p, en attente de message\n", pthread_self());

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &etat);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &etat);

    while(1){
        pause();
    }

    pthread_cleanup_pop(1);

    pthread_exit(0);
}

void handler(int sig){
    printf("\n");
    kill(getpid(), SIGUSR1);
}

void thread_handler(int sig){
    printf("thread : %p, message reçu !\n", pthread_self());
    pthread_exit(0);
}

void fctMasterThreadEtape4Fin(void *p){
    printf("master thread : %p, fin par cancel\n", pthread_self());
}