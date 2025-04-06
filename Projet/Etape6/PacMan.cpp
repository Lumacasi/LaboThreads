#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "GrilleSDL.h"
#include "Ressources.h"
#include <fcntl.h>


// Dimensions de la grille de jeu
#define NB_LIGNE 21
#define NB_COLONNE 17

// Macros utilisees dans le tableau tab
#define VIDE         0
#define MUR          1
#define PACMAN       2
#define PACGOM       3
#define SUPERPACGOM  4
#define BONUS        5
#define FANTOME      6

// Autres macros
#define LENTREE 15
#define CENTREE 8

int dir = GAUCHE;
int nbPacGom = 0;
int niveau = 1;
int delaiAttente = 300;
int score = 0;

int nbRouge = 0;
int nbVert = 0;
int nbMauve = 0;
int nbOrange = 0;

int MAJScore = 0;

typedef struct
{
    int L;
    int C;
    int couleur;
    int cache;
} S_FANTOME;

typedef struct {
    int presence;
    pthread_t tid;
} S_CASE;

S_CASE tab[NB_LIGNE][NB_COLONNE];

pthread_mutex_t mutexTab = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDir = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexNbPacGom = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condNbPacGom = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexScore = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condScore = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexNbFantome = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condNbFantome = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexDelai = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexFantomeData = PTHREAD_MUTEX_INITIALIZER;

void DessineGrilleBase();
void Attente(int milli);
void setTab(int l, int c, int presence = VIDE, pthread_t tid = 0);

void *fctThreadPacMan(void *param);
void affichagePacMan(int l, int c, int dir);
void supprimePacMan(int l, int c);

void *fctThreadEvent(void *param);

void *fctThreadPacGom(void *param);
void dessinThreadPacGom();

void *fctThreadScore(void *param);
void affichageScore(int score);

void *fctThreadBonus(void *param);

void *fctThreadCompteurFantomes(void *param);
void *fctThreadFantome(void *param);
void destructeur(void *p);

pthread_t pacman;
pthread_t pthreadEvent;
pthread_t pthreadPacGom;
pthread_t pthreadScore;
pthread_t pthreadBonus;
pthread_t pthreadCompteurFantome;
pthread_t pthreadFantomeRouge;
pthread_t pthreadFantomeVert;
pthread_t pthreadFantomeMauve;
pthread_t pthreadFantomeOrange;

pthread_key_t cle;

void threadHandler(int sig);

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
    sigset_t mask;
    struct sigaction sigAct;
    pthread_key_create(&cle, destructeur);

    int ret;

    srand((unsigned)time(NULL));

    // Ouverture de la fenetre graphique
    printf("(MAIN%p) Ouverture de la fenetre graphique\n",pthread_self()); fflush(stdout);
    if (OuvertureFenetreGraphique() < 0)
    {
        printf("Erreur de OuvrirGrilleSDL\n");
        fflush(stdout);
        exit(1);
    }

    DessineGrilleBase();

    ret = pthread_create(&pthreadPacGom, NULL, fctThreadPacGom, NULL);
    ret = pthread_create(&pacman, NULL, fctThreadPacMan, NULL);
    ret = pthread_create(&pthreadEvent, NULL, fctThreadEvent, NULL);
    ret = pthread_create(&pthreadScore, NULL, fctThreadScore, NULL);
    ret = pthread_create(&pthreadBonus, NULL, fctThreadBonus, NULL);
    ret = pthread_create(&pthreadCompteurFantome, NULL, fctThreadCompteurFantomes, NULL);

    /* 
    DessineChiffre(14,25,9);
    DessineFantome(5,9,ROUGE,DROITE);
    DessineFantomeComestible(13,15);
    DessineBonus(5,15); */

    // -------------------------------------------------------------------------
    
    Attente(10000);
    pthread_kill(pthreadFantomeRouge, SIGUSR1);
    pthread_kill(pthreadFantomeVert, SIGUSR1);
    pthread_kill(pthreadFantomeMauve, SIGUSR1);
    pthread_kill(pthreadFantomeOrange, SIGUSR1);

    ret = pthread_join(pacman, NULL);

    printf("Attente de 1500 millisecondes...\n");
    Attente(1500);

    // Fermeture de la fenetre
    printf("(MAIN %p) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
    FermetureFenetreGraphique();
    printf("OK\n"); fflush(stdout);

    pthread_key_delete(cle);
    exit(0);
}

//*********************************************************************************************
void Attente(int milli) {
    struct timespec del;
    del.tv_sec = milli/1000;
    del.tv_nsec = (milli%1000)*1000000;
    nanosleep(&del,NULL);
}

//*********************************************************************************************
void setTab(int l, int c, int presence, pthread_t tid) {
    tab[l][c].presence = presence;
    tab[l][c].tid = tid;
}

//*********************************************************************************************
void DessineGrilleBase() {
    int t[NB_LIGNE][NB_COLONNE]
    = { {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1},
    {1,1,1,1,0,1,1,0,1,0,1,1,0,1,1,1,1},
    {1,1,1,1,0,1,0,0,0,0,0,1,0,1,1,1,1},
    {1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,1},
    {0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0},
    {1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1},
    {1,1,1,1,0,1,0,0,0,0,0,1,0,1,1,1,1},
    {1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,1,1,0,1,0,1,1,0,1,1,0,1},
    {1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1},
    {1,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}};

    for (int l=0 ; l<NB_LIGNE ; l++)
        for (int c=0 ; c<NB_COLONNE ; c++) {
        if (t[l][c] == VIDE) {
            setTab(l,c);
            EffaceCarre(l,c);
        }
        if (t[l][c] == MUR) {
            setTab(l,c,MUR); 
            DessineMur(l,c);
        }
    }
}

//*********************************************************************************************

void *fctThreadPacGom(void *param) {
    int i, j;
    while (1) {
        pthread_mutex_lock(&mutexTab);
        for (i = 0; i < NB_LIGNE; i++) {
            for (j = 0; j < NB_COLONNE; j++) {
                if (tab[i][j].presence == VIDE) {
                    setTab(i, j, PACGOM);
                    DessinePacGom(i, j);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom++;
                    pthread_mutex_unlock(&mutexNbPacGom);
                }
                if (i == 15 && j == 8) {
                    setTab(15, 8, VIDE);
                    EffaceCarre(15, 8);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                }
                if (i == 8 && j == 8) {
                    setTab(8, 8, VIDE);
                    EffaceCarre(8, 8);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                }
                if (i == 9 && j == 8) {
                    setTab(9, 8, VIDE);
                    EffaceCarre(9, 8);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                }
                if (i == 2 && j == 1) {
                    setTab(2, 1, SUPERPACGOM);
                    DessineSuperPacGom(2, 1);
                }
                if (i == 2 && j == 15) {
                    setTab(2, 15, SUPERPACGOM);
                    DessineSuperPacGom(2, 15);
                }
                if (i == 15 && j == 1) {
                    setTab(15, 1, SUPERPACGOM);
                    DessineSuperPacGom(15, 1);
                }
                if (i == 15 && j == 15) {
                    setTab(15, 15, SUPERPACGOM);
                    DessineSuperPacGom(15, 15);
                }
            }
        }
        pthread_mutex_unlock(&mutexTab);
        pthread_mutex_lock(&mutexNbPacGom);
        dessinThreadPacGom();
        pthread_mutex_unlock(&mutexNbPacGom);

        pthread_mutex_lock(&mutexNbPacGom);
        while (nbPacGom > 0) {
            dessinThreadPacGom();
            pthread_cond_wait(&condNbPacGom, &mutexNbPacGom);
        }
        niveau++;
        pthread_mutex_lock(&mutexDelai);
        delaiAttente /= 2;
        pthread_mutex_unlock(&mutexDelai);
        dessinThreadPacGom();
        pthread_mutex_unlock(&mutexNbPacGom);
    }

    return NULL;
}

void dessinThreadPacGom(){
    DessineChiffre(14, 22, niveau);
    int temp = nbPacGom; 
    DessineChiffre(12, 24, temp%10);
    temp /= 10;
    DessineChiffre(12, 23, temp%10);
    temp /= 10;
    DessineChiffre(12, 22, temp);
}

void *fctThreadPacMan(void *param) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);

    struct sigaction action;
    action.sa_handler = threadHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    int l = 15, c = 8;
    int delai;

    setTab(l, c, PACMAN, pthread_self());
    DessinePacMan(l, c, GAUCHE);

    while (1) {
        sigprocmask(SIG_BLOCK, &mask, NULL);
        pthread_mutex_lock(&mutexDelai);
        delai = delaiAttente;
        pthread_mutex_unlock(&mutexDelai);
        Attente(delai); 
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        pthread_mutex_lock(&mutexDir);
        pthread_mutex_lock(&mutexTab);
        switch (dir) {
            case GAUCHE:
                if(c - 1 == -1) {
                    if(tab[l][16].presence == PACGOM){
                        pthread_mutex_lock(&mutexNbPacGom);
                        nbPacGom--;
                        pthread_mutex_unlock(&mutexNbPacGom);
                        pthread_cond_signal(&condNbPacGom);
                        pthread_mutex_lock(&mutexScore);
                        score++;
                        MAJScore = 1;
                        pthread_mutex_unlock(&mutexScore);
                        pthread_cond_signal(&condScore);
                    }
                    else if(tab[l][16].presence == BONUS){
                        pthread_mutex_lock(&mutexScore);
                        score += 20;
                        MAJScore = 1;
                        pthread_mutex_unlock(&mutexScore);
                        pthread_cond_signal(&condScore);
                    }
                    supprimePacMan(l, c);
                    c = 16;
                    affichagePacMan(l, c, dir);
                }
                else if(tab[l][c - 1].presence == MUR) 
                {
                }
                else if (tab[l][c - 1].presence == VIDE) {
                    supprimePacMan(l, c);
                    c--;
                    affichagePacMan(l, c, dir);
                }
                else if(tab[l][c - 1].presence == PACGOM){
                    supprimePacMan(l, c);
                    c--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score++;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c - 1].presence == SUPERPACGOM){
                    supprimePacMan(l, c);
                    c--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score += 5;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c - 1].presence == BONUS){
                    supprimePacMan(l, c);
                    c--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexScore);
                    score += 20;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c - 1].presence == FANTOME){
                    pthread_cancel(pacman);
                }
                break;
            case DROITE:
                if(c + 1 == 17) {
                if(tab[l][0].presence == PACGOM){
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score++;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][0].presence == BONUS){
                    pthread_mutex_lock(&mutexScore);
                    score += 20;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                supprimePacMan(l, c);
                c = 0;
                affichagePacMan(l, c, dir);
                }
                else if (tab[l][c + 1].presence == MUR) 
                {
                }
                else if (tab[l][c + 1].presence == VIDE) {
                    supprimePacMan(l, c);
                    c++;
                    affichagePacMan(l, c, dir);
                }
                else if(tab[l][c + 1].presence == PACGOM){
                    supprimePacMan(l, c);
                    c++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score++;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c + 1].presence == SUPERPACGOM){
                    supprimePacMan(l, c);
                    c++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score += 5;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c + 1].presence == BONUS){
                    supprimePacMan(l, c);
                    c++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexScore);
                    score += 20;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l][c + 1].presence == FANTOME){
                    pthread_cancel(pacman);
                }
                break;
            case HAUT:
                if (tab[l - 1][c].presence == MUR) 
                {
                }
                else if (tab[l - 1][c].presence == VIDE) {
                    supprimePacMan(l, c);
                    l--;
                    affichagePacMan(l, c, dir);
                }
                else if(tab[l - 1][c].presence == PACGOM){
                    supprimePacMan(l, c);
                    l--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score++;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l - 1][c].presence == SUPERPACGOM){
                    supprimePacMan(l, c);
                    l--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score += 5;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l - 1][c].presence == BONUS){
                    supprimePacMan(l, c);
                    l--;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexScore);
                    score += 20;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l - 1][c].presence == FANTOME){
                    pthread_cancel(pacman);
                }
                break;
            case BAS:
                if (tab[l + 1][c].presence == MUR) 
                {
                }
                else if (tab[l + 1][c].presence == VIDE) {
                    supprimePacMan(l, c);
                    l++;
                    affichagePacMan(l, c, dir);
                }
                else if(tab[l + 1][c].presence == PACGOM){
                    supprimePacMan(l, c);
                    l++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score++;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l + 1][c].presence == SUPERPACGOM){
                    supprimePacMan(l, c);
                    l++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexNbPacGom);
                    nbPacGom--;
                    pthread_mutex_unlock(&mutexNbPacGom);
                    pthread_cond_signal(&condNbPacGom);
                    pthread_mutex_lock(&mutexScore);
                    score += 5;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l + 1][c].presence == BONUS){
                    supprimePacMan(l, c);
                    l++;
                    affichagePacMan(l, c, dir);
                    pthread_mutex_lock(&mutexScore);
                    score += 20;
                    MAJScore = 1;
                    pthread_mutex_unlock(&mutexScore);
                    pthread_cond_signal(&condScore);
                }
                else if(tab[l + 1][c].presence == FANTOME){
                    pthread_cancel(pacman);
                }
                break;
        }
        pthread_mutex_unlock(&mutexTab);
        pthread_mutex_unlock(&mutexDir);
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
    }
}
void affichagePacMan(int l, int c, int dir){
    setTab(l,c,PACMAN,pthread_self());
    DessinePacMan(l,c,dir);
}

void supprimePacMan(int l, int c){
    setTab(l,c);
    EffaceCarre(l,c);
}

void *fctThreadEvent(void *param) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    EVENT_GRILLE_SDL event;
    char ok;
    ok = 0;
    while (!ok) {
        event = ReadEvent();
        if (event.type == CROIX) ok = 1;
        if (event.type == CLAVIER) {
        switch (event.touche) {
            case 'q':
                ok = 1;
                break;
            case KEY_RIGHT:
                printf("Fleche droite !\n");
                pthread_kill(pacman, SIGHUP);
                break;
            case KEY_LEFT:
                printf("Fleche gauche !\n");
                pthread_kill(pacman, SIGINT);
                break;
            case KEY_UP:
                printf("Fleche haut !\n");
                pthread_kill(pacman, SIGUSR1);
                break;
            case KEY_DOWN:
                printf("Fleche bas !\n");
                pthread_kill(pacman, SIGUSR2);
                break;
            }
        }
    }
    pthread_exit(NULL);
}

void *fctThreadScore(void *param) {
    while (1) {
        pthread_mutex_lock(&mutexScore);
        while (MAJScore == 0) {
            pthread_cond_wait(&condScore, &mutexScore);
        }
        affichageScore(score);
        MAJScore = 0;
        pthread_mutex_unlock(&mutexScore);
    }
    return NULL;
}

void affichageScore(int score){
    DessineChiffre(16, 25, score%10);
    score /= 10;
    DessineChiffre(16, 24, score%10);
    score /= 10;
    DessineChiffre(16, 23, score%10);
    score /= 10;
    DessineChiffre(16, 22, score);
}

void *fctThreadBonus(void *param){
    int i, j, temps;
    while(1){
        temps = rand() % 10000 + 10000;
        Attente(temps);
        pthread_mutex_lock(&mutexTab);
        do{
            i = rand() % 18 + 1;
            j = rand() % 14 + 1;
            pthread_mutex_unlock(&mutexTab);
            printf("Problème de placement de bonus : %d %d\n", i, j);
            pthread_mutex_lock(&mutexTab);
        } while(tab[i][j].presence != VIDE || tab[i][j].presence == MUR ||
        (i == 9 && j == 8) ||
        (i == 8 && j == 8)
        );
        setTab(i, j, BONUS);
        DessineBonus(i, j);
        pthread_mutex_unlock(&mutexTab);
        
        Attente(10000);

        pthread_mutex_lock(&mutexTab);
        if(tab[i][j].presence == BONUS){
            setTab(i, j, VIDE);
            EffaceCarre(i, j);
        }
        pthread_mutex_unlock(&mutexTab);
    }
}

void *fctThreadCompteurFantomes(void *param) {

    while (1) {
        pthread_mutex_lock(&mutexNbFantome);
        while (nbRouge == 2 && nbVert == 2 && nbMauve == 2 && nbOrange == 2) {
            pthread_cond_wait(&condNbFantome, &mutexNbFantome);
        }

        S_FANTOME *fantome = (S_FANTOME*)malloc(sizeof(S_FANTOME));

        fantome->L = 9;
        fantome->C = 8;
        fantome->cache = 0;

        pthread_mutex_lock(&mutexFantomeData);
        if (nbRouge < 2) {
            fantome->couleur = ROUGE;
            pthread_create(&pthreadFantomeRouge, NULL, fctThreadFantome, (void *)fantome);
            nbRouge++;
        } else if (nbVert < 2) {
            fantome->couleur = VERT;
            pthread_create(&pthreadFantomeVert, NULL, fctThreadFantome, (void *)fantome);
            nbVert++;
        } else if (nbMauve < 2) {
            fantome->couleur = MAUVE;
            pthread_create(&pthreadFantomeMauve, NULL, fctThreadFantome, (void *)fantome);
            nbMauve++;
        } else if (nbOrange < 2) {
            fantome->couleur = ORANGE;
            pthread_create(&pthreadFantomeOrange, NULL, fctThreadFantome, (void *)fantome);
            nbOrange++;
        }


        pthread_mutex_unlock(&mutexNbFantome);
    }
}

void *fctThreadFantome(void *param) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    S_FANTOME *fantome = (S_FANTOME *)param;

    // Set the thread-specific data for this thread
    pthread_setspecific(cle, fantome);

    if (fantome == NULL) {
        fprintf(stderr, "Error: Thread-specific data not set for ghost thread.\n");
        return NULL;
    }

    int l = fantome->L;
    int c = fantome->C;
    int couleur = fantome->couleur;
    printf("couleur : %d\n", couleur);
    int cache = fantome->cache;
    
    int dir = HAUT;
    
    int prochaineL;
    int prochaineC;
    
    int tempCache;
    pthread_mutex_lock(&mutexTab);
    do{
        pthread_mutex_unlock(&mutexTab);
        pthread_mutex_lock(&mutexDelai);
        int delai = 5*delaiAttente/3;
        pthread_mutex_unlock(&mutexDelai);
        Attente(delai);
        pthread_mutex_lock(&mutexTab);
    } while(tab[l][c].presence != VIDE);
    pthread_mutex_unlock(&mutexFantomeData);
    pthread_mutex_unlock(&mutexTab);

    pthread_mutex_lock(&mutexTab);
    setTab(fantome->L, fantome->C, FANTOME, pthread_self());
    pthread_mutex_unlock(&mutexTab);
    DessineFantome(fantome->L, fantome->C, fantome->couleur, dir);

    pthread_mutex_lock(&mutexDelai);
    int delai = 5*delaiAttente / 3;
    pthread_mutex_unlock(&mutexDelai);
    Attente(delai);

    while(1){

        prochaineL = l;
        prochaineC = c;

        switch (dir) {
            case HAUT:
                prochaineL = l - 1;
                break;
            case BAS:
                prochaineL = l + 1;
                break;
            case GAUCHE:
                prochaineC = c - 1;
                break;
            case DROITE:
                prochaineC = c + 1;
                break;
        }
        pthread_mutex_lock(&mutexTab);

        if(tab[prochaineL][prochaineC].presence == PACMAN){
            pthread_cancel(pacman);
        }

        if(tab[prochaineL][prochaineC].presence == VIDE || 
        tab[prochaineL][prochaineC].presence == PACGOM ||
        tab[prochaineL][prochaineC].presence == SUPERPACGOM ||
        tab[prochaineL][prochaineC].presence == BONUS) 
        {
            tempCache = cache;

            if (tab[prochaineL][prochaineC].presence == VIDE)
                cache = VIDE;
            else if (tab[prochaineL][prochaineC].presence == PACGOM)
                cache = PACGOM;
            else if (tab[prochaineL][prochaineC].presence == SUPERPACGOM)
                cache = SUPERPACGOM; 
            else if (tab[prochaineL][prochaineC].presence == BONUS)
                cache = BONUS;
            
            
            
            setTab(l, c, tempCache);
            if (tempCache == PACGOM) {
                DessinePacGom(l, c);
            } else if (tempCache == SUPERPACGOM) {
                DessineSuperPacGom(l, c);
            } else if (tempCache == BONUS) {
                DessineBonus(l, c);
            } else if (tempCache == VIDE) {
                EffaceCarre(l, c);
            }
            setTab(prochaineL, prochaineC, FANTOME, pthread_self());
            DessineFantome(prochaineL, prochaineC, couleur, dir);
            pthread_mutex_unlock(&mutexTab);
            l = prochaineL;
            c = prochaineC;
            
        }

        else if(tab[prochaineL][prochaineC].presence == MUR ||
        tab[prochaineL][prochaineC].presence == FANTOME)
        {

            int tabChoix[4] = {0};
            int dirValides = 0;

            if (l - 1 >= 0 && (tab[l - 1][c].presence == VIDE ||
            tab[l - 1][c].presence == PACGOM ||
            tab[l - 1][c].presence == SUPERPACGOM ||
            tab[l - 1][c].presence == BONUS)) 
            {
                tabChoix[dirValides++] = HAUT;
            }
            if (l + 1 < NB_LIGNE && (tab[l + 1][c].presence == VIDE ||
            tab[l + 1][c].presence == PACGOM ||
            tab[l + 1][c].presence == SUPERPACGOM ||
            tab[l + 1][c].presence == BONUS))
            {
                tabChoix[dirValides++] = BAS; 
            }
            if (c - 1 >= 0 && (tab[l][c - 1].presence == VIDE ||
            tab[l][c - 1].presence == PACGOM ||
            tab[l][c - 1].presence == SUPERPACGOM ||
            tab[l][c - 1].presence == BONUS)) 
            {
                tabChoix[dirValides++] = GAUCHE;
            }
            if (c + 1 < NB_COLONNE && (tab[l][c + 1].presence == VIDE ||
            tab[l][c + 1].presence == PACGOM ||
            tab[l][c + 1].presence == SUPERPACGOM ||
            tab[l][c + 1].presence == BONUS)) 
            {
                tabChoix[dirValides++] = DROITE;
            }

            if(dirValides > 0) {
                dir = tabChoix[rand() % dirValides];
            }

            pthread_mutex_unlock(&mutexTab);

            continue;

        }
        pthread_mutex_lock(&mutexDelai);
        int delai = 5*delaiAttente / 3;
        pthread_mutex_unlock(&mutexDelai);
        Attente(delai);
    }

return NULL;
}

void threadHandler(int sig) {
    switch(sig) {
        case SIGHUP:
            printf("SIGHUP reçu dans le thread %p\n", pthread_self());
            dir = DROITE;
            break;
        case SIGINT:
            printf("SIGINT reçu dans le thread %p\n", pthread_self());
            dir = GAUCHE;
            break;
        case SIGUSR1:
            printf("SIGUSR1 reçu dans le thread %p\n", pthread_self());
            dir = HAUT;
            break;
        case SIGUSR2:
            printf("SIGUSR2 reçu dans le thread %p\n", pthread_self());
            dir = BAS;
        break;
    }
}

void destructeur(void *p){
    free(p);
}