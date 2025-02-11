#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void *fctThreadEtape3Thread(void *param);

struct threadData{
    char nomFichier[18] = "fichierThread.txt";
    char param[10];
    int tabs;
    int compteurTrouve;
};

pthread_t threadHandle1, threadHandle2, threadHandle3, threadHandle4;

int main(){
    int ret, *retThread1, *retThread2, *retThread3, *retThread4;
    struct threadData data1, data2, data3, data4;
    strcpy(data1.param, "printf");
    strcpy(data2.param, "ret");
    strcpy(data3.param, "void");
    strcpy(data4.param, "int");

    data1.tabs = 0;
    data2.tabs = 1;
    data3.tabs = 2;
    data4.tabs = 3;

    ret = pthread_create(&threadHandle1, NULL, (void *(*) (void *))fctThreadEtape3Thread, &data1);
    ret = pthread_create(&threadHandle2, NULL, (void *(*) (void *))fctThreadEtape3Thread, &data2);
    ret = pthread_create(&threadHandle3, NULL, (void *(*) (void *))fctThreadEtape3Thread, &data3);
    ret = pthread_create(&threadHandle4, NULL, (void *(*) (void *))fctThreadEtape3Thread, &data4);
    
    ret = pthread_join(threadHandle1, (void**)&retThread1);
    ret = pthread_join(threadHandle2, (void**)&retThread2);
    ret = pthread_join(threadHandle3, (void**)&retThread3);
    ret = pthread_join(threadHandle4, (void**)&retThread4);

    printf("Valeurs retournées par les threads : %d, %d, %d, %d", *retThread1, *retThread2, *retThread3, *retThread4);

    free(retThread1);
    free(retThread2);
    free(retThread3);
    free(retThread4);

    return 0;
}

void *fctThreadEtape3Thread(void *param) {
    struct threadData *data = (struct threadData *)param;
    int *retour = (int *)malloc(sizeof(int));
    *retour = 0;
    int taille = strlen(data->param);
    char *buf = (char *)malloc(taille + 1);
    int i = 0;
    int ret;
    int boucle = 1;

    while (boucle) {
        for (int j = 0; j < data->tabs; j++) {
            printf("\t");
        }
        printf("*\n");
        int descripteur = open(data->nomFichier, O_RDONLY);
        if (descripteur == -1) {
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        if (read(descripteur, buf, taille) < taille) {
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            buf[taille] = '\0';
            if (strcmp(buf, data->param) == 0) {
                (*retour)++;
            }
            i++;
        }
    }
    free(buf);
    pthread_exit(retour);
    return 0;
}