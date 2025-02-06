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
    strcpy(data1.param, "salut");
    strcpy(data2.param, "coucou");
    strcpy(data3.param, "yoyo");
    strcpy(data4.param, "hello");

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

    return 0;
}

void *fctThreadEtape3Thread(void *param){
    struct threadData *data = (struct threadData *)param;
    int i = 0, boucle = 1, retRead, tabulations;
    int trouve = 0;
    char paramEff[10], lecture[10];
    strcpy(paramEff, data->param);
    data->compteurTrouve = 0;
    while(boucle){
        for(tabulations = 0; tabulations < data->tabs; tabulations++){
            printf("\t");
        }
        printf("*\n");
        int descripteur = open(data->nomFichier, O_RDONLY);
        if(descripteur == -1){
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        int lenstr = strlen(paramEff);
        retRead = read(descripteur, &lecture, lenstr);
        if(retRead == 0){
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            if(strcmp(lecture, paramEff) == 0)
                data->compteurTrouve++;
            i++;
        }
    }
    trouve = data->compteurTrouve;
    pthread_exit((void *) &trouve);
}