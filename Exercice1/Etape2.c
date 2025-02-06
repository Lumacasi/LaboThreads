#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void *fctThreadEtape2Thread1(int *param);
void *fctThreadEtape2Thread2(int *param);
void *fctThreadEtape2Thread3(int *param);
void *fctThreadEtape2Thread4(int *param);

pthread_t threadHandle1, threadHandle2, threadHandle3, threadHandle4;

int main(){
    int ret, *retThread1, *retThread2, *retThread3, *retThread4;

    ret = pthread_create(&threadHandle1, NULL, (void *(*) (void *))fctThreadEtape2Thread1, NULL);
    ret = pthread_create(&threadHandle2, NULL, (void *(*) (void *))fctThreadEtape2Thread2, NULL);
    ret = pthread_create(&threadHandle3, NULL, (void *(*) (void *))fctThreadEtape2Thread3, NULL);
    ret = pthread_create(&threadHandle4, NULL, (void *(*) (void *))fctThreadEtape2Thread4, NULL);
    
    ret = pthread_join(threadHandle1, (void**)&retThread1);
    ret = pthread_join(threadHandle2, (void**)&retThread2);
    ret = pthread_join(threadHandle3, (void**)&retThread3);
    ret = pthread_join(threadHandle4, (void**)&retThread4);

    printf("Valeurs retournées par les threads : %d, %d, %d, %d", *retThread1, *retThread2, *retThread3, *retThread4);

    return 0;
}

void *fctThreadEtape2Thread1(int *param){
    int i = 0, boucle = 1;
    static int compteurTrouve = 0;
    char paramEff[6] = {"salut"}, lecture[6];
    while(boucle){
        puts("*");
        int descripteur = open("fichierThread.txt", O_RDONLY);
        if(descripteur == -1){
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        int lenstr = strlen(paramEff);
        if(read(descripteur, &lecture, lenstr) == 0){
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            if(strcmp(lecture, paramEff) == 0)
                compteurTrouve++;
            i++;
        }
    }
    pthread_exit(&compteurTrouve);
}

void *fctThreadEtape2Thread2(int *param){
    int i = 0, boucle = 1;
    static int compteurTrouve = 0;
    char paramEff[7] = {"coucou"}, lecture[7];
    while(boucle){
        puts("\t*");
        int descripteur = open("fichierThread.txt", O_RDONLY);
        if(descripteur == -1){
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        int lenstr = strlen(paramEff);
        if(read(descripteur, &lecture, lenstr) == 0){
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            if(strcmp(lecture, paramEff) == 0)
                compteurTrouve++;
            i++;
        }
    }
    pthread_exit(&compteurTrouve);
}

void *fctThreadEtape2Thread3(int *param){
    int i = 0, boucle = 1;
    static int compteurTrouve = 0;
    char paramEff[5] = {"yoyo"}, lecture[5];
    while(boucle){
        puts("\t\t*");
        int descripteur = open("fichierThread.txt", O_RDONLY);
        if(descripteur == -1){
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        int lenstr = strlen(paramEff);
        if(read(descripteur, &lecture, lenstr) == 0){
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            if(strcmp(lecture, paramEff) == 0)
                compteurTrouve++;
            i++;
        }
    }
    pthread_exit(&compteurTrouve);
}

void *fctThreadEtape2Thread4(int *param){
    int i = 0, boucle = 1;
    static int compteurTrouve = 0;
    char paramEff[6] = {"hello"}, lecture[6];
    while(boucle){
        puts("\t\t\t*");
        int descripteur = open("fichierThread.txt", O_RDONLY);
        if(descripteur == -1){
            printf("Erreur lors de l'ouverture du fichier\n");
            pthread_exit(0);
        }
        lseek(descripteur, i, SEEK_SET);
        int lenstr = strlen(paramEff);
        if(read(descripteur, &lecture, lenstr) == 0){
            close(descripteur);
            boucle = 0;
        } else {
            close(descripteur);
            if(strcmp(lecture, paramEff) == 0)
                compteurTrouve++;
            i++;
        }
    }
    pthread_exit(&compteurTrouve);
}