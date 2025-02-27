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

void *fctThreadEtape1(void *param);

int main(){

    DONNEES data[] = {{"MATAGNE",15},{"WILVERS",10},{"WAGNER",17},{"QUETTIER",8},{"",0}};
    DONNEES Param;
    pthread_t threadHandles[10];
    int i = 0;

    while (strcmp(data[i].nom, "") != 0 && data[i].nbSecondes != 0) {
        memcpy(&Param, &data[i], sizeof(DONNEES));
        pthread_create(&threadHandles[i], NULL, fctThreadEtape1, (void *)&Param);
        i++;
    }

    for (int j = 0; j < i; j++) {
        pthread_join(threadHandles[j], NULL);
    }

    exit(0);
}

void *fctThreadEtape1(void *param) {
    DONNEES *data = (DONNEES *)param;

    printf("Thread %d.%d lancé\n", getpid(), pthread_self());

    printf("%s\n", data->nom);

    sleep(data->nbSecondes);

    printf("Thread %d.%d terminé\n", getpid(), pthread_self());

    return NULL;
}