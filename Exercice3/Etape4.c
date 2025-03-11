#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

typedef struct{
    char nom[20];
    int nbSecondes;
} DONNEES;

int threadCompteur = 0;

void *fctThreadEtape4(void *param);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNombreThread = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condNombreThread;

pthread_key_t cle;

void destructeur(void *p);

void threadHandler(int sig);

int main(){
    
    DONNEES data[] = {{"MATAGNE",15},{"WILVERS",10},{"WAGNER",17},{"QUETTIER",8},{"",0}};
    DONNEES Param;
    pthread_t threadHandles[10];
    pthread_cond_init(&condNombreThread, NULL);
    pthread_key_create(&cle, destructeur);

    struct sigaction action;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    while (strcmp(data[threadCompteur].nom, "") != 0 && data[threadCompteur].nbSecondes != 0) {
        pthread_mutex_lock(&mutex);
        memcpy(&Param, &data[threadCompteur], sizeof(DONNEES));
        pthread_create(&threadHandles[threadCompteur], NULL, fctThreadEtape4, (void *)&Param);
        threadCompteur++;
    }

    pthread_mutex_lock(&mutexNombreThread);
    while(threadCompteur){
        pthread_cond_wait(&condNombreThread, &mutexNombreThread);
    }
    pthread_mutex_unlock(&mutexNombreThread);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexNombreThread);
    pthread_cond_destroy(&condNombreThread);
    pthread_key_delete(cle);
    exit(0);
}

void *fctThreadEtape4(void *param) {
    DONNEES *dataThread = (DONNEES *)malloc(sizeof(DONNEES));
    memcpy(dataThread, param, sizeof(DONNEES));
    int nbSecondesRestantes, retourNano;
    struct sigaction action;
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

    struct sigaction thread_action;
    thread_action.sa_handler = threadHandler;
    sigemptyset(&thread_action.sa_mask);
    thread_action.sa_flags = 0;
    sigaction(SIGINT, &thread_action, NULL);

    pthread_setspecific(cle, dataThread);

    printf("Thread %d.%d lancé\n", getpid(), pthread_self());
    
    pthread_mutex_unlock(&mutex);

    struct timespec ts;
    ts.tv_sec = dataThread->nbSecondes;
    ts.tv_nsec = 0;
    struct timespec ts2;

    while((retourNano = nanosleep(&ts, &ts2)) != 0){
        if(retourNano == -1){
            ts = ts2;
    }}
    
    printf("Thread %d.%d terminé\n", getpid(), pthread_self());

    pthread_mutex_lock(&mutexNombreThread);
    threadCompteur--;    
    pthread_mutex_unlock(&mutexNombreThread);
    pthread_cond_signal(&condNombreThread);

    return NULL;
}

void threadHandler(int sig){
    DONNEES *dataThread = (DONNEES *)pthread_getspecific(cle);
    printf("Thread %d.%d s'occupe de %s\n", getpid(), pthread_self(), dataThread->nom);
}

void destructeur(void *p){
    free(p);
}