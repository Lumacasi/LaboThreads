#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void *fctThreadEtape1(int *param);

pthread_t threadHandle;

int main(){
    int ret, *retThread;

    ret = pthread_create(&threadHandle, NULL, (void *(*) (void *))fctThreadEtape1, NULL);    
    puts("Thread secondaire lancé\n");
    puts("Attente de la fin du thread secondaire");
    ret = pthread_join(threadHandle, (void**)&retThread);
    printf("Valeur renvoyée par le thread secondaire = %d\n", *retThread);
    return 0;
    

}

void *fctThreadEtape1(int *param){
    int i = 0, boucle = 1;
    static int compteurTrouve = 0;
    char paramEff[7] = {"printf"}, lecture[7];
    while(boucle){
        puts("*");
        int descripteur = open("commands.txt", O_RDONLY);
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