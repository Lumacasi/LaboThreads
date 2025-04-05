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

void *fctThreadEtape3(void *param);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNombreThread = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condNombreThread;

int main(){

    DONNEES data[] = {{"MATAGNE",15},{"WILVERS",10},{"WAGNER",17},{"QUETTIER",8},{"",0}};
    DONNEES Param;
    pthread_t threadHandles[10];
    pthread_cond_init(&condNombreThread, NULL);

    while (strcmp(data[threadCompteur].nom, "") != 0 && data[threadCompteur].nbSecondes != 0) {
        pthread_mutex_lock(&mutex);
        memcpy(&Param, &data[threadCompteur], sizeof(DONNEES));
        pthread_create(&threadHandles[threadCompteur], NULL, fctThreadEtape3, (void *)&Param);
        threadCompteur++;
    }

    pthread_mutex_lock(&mutexNombreThread);
    while(threadCompteur)
        pthread_cond_wait(&condNombreThread, &mutexNombreThread);
    pthread_mutex_unlock(&mutexNombreThread);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condNombreThread);
    exit(0);
}

void *fctThreadEtape3(void *param) {
    DONNEES *data = (DONNEES *)param;

    printf("Thread %d.%d lancé\n", getpid(), pthread_self());
    printf("%s\n", data->nom);

    pthread_mutex_unlock(&mutex);

    struct timespec ts;
    ts.tv_sec = data->nbSecondes;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);
    
    printf("Thread %d.%d terminé\n", getpid(), pthread_self());

    pthread_mutex_lock(&mutexNombreThread);
    threadCompteur--;    
    pthread_mutex_unlock(&mutexNombreThread);
    pthread_cond_signal(&condNombreThread);

    return NULL;
}