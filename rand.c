/*
ERDENEBAATAR Munkhdorj
RAHMOUNI Hajar
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#ifndef NBFILS
#define NBFILS 4
#endif

//Méthode pour calculer la somme des valeurs
unsigned long long somme(unsigned int *tab, unsigned long taille){
    unsigned long long s=0;
    for (unsigned long long i=0; i<taille; ++i) {
        s+=tab[i];
    }
    return s;
}

//Méthode pour chercher la valeur maximum du tableau
int chercherMax(unsigned int *tab, unsigned long taille){
    unsigned int max=0;
    for (unsigned long i=1; i<taille; ++i) {
        if( max < tab[i]){
            max = tab[i];
        }
    }
    return max;
}

//Méthode pour chercher la valeur minimum du tableau
int chercherMin(unsigned int *tab, unsigned long taille){
    unsigned int min=tab[0];
    for (unsigned long i=1; i<taille; ++i) {
        if( min > tab[i]){
            min = tab[i];
        }
    }
    return min;
}

//Calcul d'équilibre de la fonction rand(), si la valeur minimum et la valeur maximum ne dépassent pas margeMin et margeMax alors rand est équilibrée
void calculEquilibre(unsigned int *tab, unsigned long taille, unsigned long alea){
    unsigned long margeMin = ((unsigned long)alea/taille)*0.9;
    unsigned long margeMax = ((unsigned long)alea/taille)*1.1;
    //on cherche valeur max et min dans le tableau courant
    int max = chercherMax(tab,taille);
    int min = chercherMin(tab,taille);

    if(max > margeMax){
        printf("\n### rand() non équilibrée, %u > margeMax = %ld ###\n", max, margeMax);
    }
    else if(min < margeMin){
        printf("\n### rand() non équilibrée, %u < margeMin = %ld ###\n", min, margeMin);
    }else{
        printf("\nrand() EQUILIBREE, MARGE[%ld, %ld],ValeurMinMaxDeTab[%d, %d]\n", margeMin, margeMax, min, max);
    }
}

//méthode pour afficher le tableau
void afficher(unsigned int * tab, unsigned long taille){
    printf("\n Affichage du resultat, On est dans le PERE \n");
    for (unsigned long i=0; i<taille; i++) {
        printf("|%u", tab[i]);
    }
    printf("\n");
}

int  main(){
    int shmID;
    unsigned long alea;
    unsigned long long i,rang, s=0;
    unsigned int *tab;
    unsigned long taille;

    //Récupération des nombres de processus, taille du tableau et nombre de valeurs à générer
    pid_t fils[NBFILS];
    printf("Entrez le taille du tableau \n");
    (void)scanf("%lu", &taille);;
    printf("Entrez le nombre de valeurs à générer\n");
    (void)scanf("%lu", &alea);


    //On renvoie l'identifiant du segment de mémoire partagée dans l'entier shmID
    shmID = shmget(IPC_PRIVATE, taille*sizeof(int), IPC_CREAT | 0666);
    //Si shmID == -1
    if (shmID < 0) {
        printf("shmget ERROR\n");
        exit(1);
    }
    //On attache le segment de mémoire partagée identifié par shmID au pointeur du tableau
    tab = (unsigned int *) shmat(shmID, NULL, 0);

    //Variables pour les processus à créer
    int pid,status;
    for(s=0; s < alea;){
        for (int p = 0; p < NBFILS && s < alea; p++) {
            unsigned long interMin=p*taille/NBFILS;
            unsigned long interMax=interMin+(taille/NBFILS);
            switch(pid = fils[p]= fork()){
                //création de fils a échoué
                case -1:
                perror("FORK ERROR");
                exit(1);

                //dans le fils
                case 0:
                srand(getpid());
                for (i=0; i<+10000; i++) {
                    tab[rand()%(unsigned int)interMax+interMin]++;
                }
                exit(p);

                //dans le pere
                default:
                waitpid(fils[p], &status, 0); //attend le fils

                //Mettre à jour le tableau après les 10000 valeurs générées
                //calcul equilibre par le pere
                s=s+10000;
                if(s%(NBFILS*10000)==0 && s>0){
                    afficher(tab,taille);
                    calculEquilibre(tab,taille,somme(tab,taille));
                    //mise à jour du tableau toujours à la même zone mémoire
                    shmID = shmget(IPC_PRIVATE, taille*sizeof(int), IPC_CREAT | 0666);
                    tab = (unsigned int *) shmat(shmID, NULL, 0);
                }
                break;
            }
        }
    }
    // Détachement de la zone mémoire du pointeur de tableau
    shmdt((void *) tab);
    shmctl(shmID, IPC_RMID, NULL);
    exit(0);
}
