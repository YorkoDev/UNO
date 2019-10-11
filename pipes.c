#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#include "pipes.h"
#include "archivos.h"
#include "juego.h"

int id;
int turnos[4];


//[padre,hijo1.hijo2,hijo3]
int creacionprocesos(){
    int i;
    turnos[0]= getpid();
    i = fork();
    if(i < 0){
        return -1;
    }
    else if(i > 0){
        turnos[1]= i;
        i = fork();
        if(i < 0){
            return -2;
        }
        else if(i > 0){
            turnos[2]=i;
            i = fork();
            if(i < 0){
                return -3;
            }
            else if(i > 0) turnos[3] = i;
            else turnos[3] = getpid();
            return 1;   
        }
        else{
            turnos[3] = -1;
            turnos[2] = getpid();
            return 1;
        }
    }
    else{
        turnos[3] = -1;
        turnos[2] = -1;
        turnos[1] = getpid();
        return 1;
    }
}

void jugando(){
	int p,i;
    int j;
    char mov[30];
    char poz[30];
    char* carta;
	char** cartas;
	char** poso;
	char col='z';
	int* H1aP;
	H1aP =(int*)malloc(sizeof(int)*2);
	int* H2aP;
	H2aP =(int*)malloc(sizeof(int)*2);
	int* H3aP;
	H3aP =(int*)malloc(sizeof(int)*2);
	int* PaH1;
	PaH1 =(int*)malloc(sizeof(int)*2);
	int* PaH2;
	PaH2 =(int*)malloc(sizeof(int)*2);
	int* PaH3;
	PaH3 =(int*)malloc(sizeof(int)*2);
	char paramandar[100];
	char pararecibir[100];
	
	pipe(H1aP); //H1 a padre
	pipe(H2aP); //H2 a padre
	pipe(H3aP); //H3 a padre
	pipe(PaH1); //padre a H1
	pipe(PaH2); //padre a H2
	pipe(PaH3); //padre a H3
	
	p = creacionprocesos();

	p = getpid();
	if(p == turnos[0]){
		char CJTA[11];
		strcpy(CJTA," ");
		i = 0;
		char turn[2];
		//Cerrando lectura de padre a hijo
		close(PaH1[0]);
		close(PaH2[0]);
		close(PaH3[0]);

		//Cerrando escritura de hijo a padre
		close(H1aP[1]);
		close(H2aP[1]);
		close(H3aP[1]);
		
		strcpy(paramandar,"1");

		while(strcmp(paramandar,"1")==0){
			strcpy(turn,"");
			sprintf(turn,"%d",i);
			write(PaH1[1],turn,strlen(turn)+1);
			write(PaH2[1],turn,strlen(turn)+1);
			write(PaH3[1],turn,strlen(turn)+1);
			if(i == 0){
				cartas=obtenercartas("mano1");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 1\n");
				if(strcmp(CJTA," ") != 0) printf("Jugador 1: Jugador 4 me jugo %s\n",CJTA);
				printf("Jugador 1: Mmmmmmm que carta jugare ahora?\n");
				printf("[109] Robar carta y saltar\n");
				print(cartas);
				printf("La carta en el pozo es:\n");
				print(poso);

				scanf("%d",&j);

				while(j != 109 && !(puedojugarla(cartas[j],col)))
				{
					printf("Carta invalida, porfavor elegir otra carta\n");
					print(cartas);
					printf("La carta en el pozo es:\n");
					print(poso);
					scanf("%d",&j);
				}
				if (j != 109)
				{
					printf("ctm\n");
					strcpy(mov, "mano1/");
					strcat(mov, cartas[j]);

					strcpy(poz, "pozo/");
					strcat(poz, poso[0]);
					eliminar_carta(poz);
					strcpy(poz, "pozo/");
					strcat(poz, cartas[j]);
					mover_carta(mov, poz);

					write(PaH1[1],cartas[j],strlen(cartas[j])+1);
				}
				else
				{

					carta = cartaMazo();
					if(puedojugarla(carta,col)){
						printf("Jugador 1: Wenaaa la buena suerte esta de mi lado %s\n",carta);
						strcpy(mov, "mano1/");
						strcat(mov, carta);

						strcpy(poz, "pozo/");
						strcat(poz, poso[0]);
						eliminar_carta(poz);
						strcpy(poz, "pozo/");
						strcat(poz, carta);
						mover_carta(mov, poz);	
						write(PaH1[1],carta,100);
						free(carta);					
					}
					else{
						printf("Jugador 1: Ay no que mala pata!\n");
						char None[5];
						strcpy(None,"None");
						printf("None %s\n",None);
						write(PaH1[1],None,5);
						
					}
				}
				liberarmemoria(cartas);
				liberarmemoria(poso);

			}
			else if (i == 1){
				while((read(H1aP[0],pararecibir,100))<0);
				printf("Jugador 1: Asi que el Jugador 2 puso un %s... Suerte con eso Jugador 3\n",pararecibir);
				write(PaH2[1],pararecibir,strlen(pararecibir)+1);
			}
			else if (i == 2){
				while((read(H2aP[0],pararecibir,100))<0);
				printf("Jugador 1: Asi que el Jugador 3 puso un %s... Suerte con eso Jugador 4\n",pararecibir);
				write(PaH3[1],pararecibir,strlen(pararecibir)+1);
			}
			else if (i == 3){
				while((read(H3aP[0],pararecibir,100))<0);
				printf("Jugador 1: Asi que el Jugador 4 puso un %s... Suerte con eso Jugador 1, oh cresta soy yo\n",pararecibir);
				strcpy(CJTA,pararecibir);
			}
			i = (i + 1)%4;
			sleep(1);
			if(strcmp(obtenercartas("mazo")[0],"STOP") == 0) strcpy(paramandar,"-1");
		}
		strcpy(turn,"");
		sprintf(turn,"%d",5);
		write(PaH1[1],turn,strlen(turn)+1);
		write(PaH2[1],turn,strlen(turn)+1);
		write(PaH3[1],turn,strlen(turn)+1);
	
		close(PaH1[1]);
		close(PaH2[1]);
		close(PaH3[1]);
		close(H1aP[0]);
		close(H2aP[0]);
		close(H3aP[0]);
	}
	else if(p == turnos[1]){
		char CJTA[11]; //Carta jugada turno anterior
		//Cerrando escritura de padre a hijo
		close(PaH1[1]);

		//Cerrando lectura de hijo a padre
		close(H1aP[0]);

		//Cerrando cosas que no usara el proceso 1 saludos
		close(PaH2[0]);
		close(PaH3[0]);
		close(H2aP[1]);
		close(H3aP[1]);
		close(PaH2[1]);
		close(PaH3[1]);
		close(H2aP[0]);
		close(H3aP[0]);

		while(1){
			while((read(PaH1[0],pararecibir,100))<0);
			if(strcmp(pararecibir,"5")== 0) break;
			if(strcmp(pararecibir,"1") == 0){
                cartas=obtenercartas("mano2");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 2\n");
				printf("Jugador 2: Jugador 1 me jugo %s\n",CJTA);
				printf("Jugador 2: Mmmmmmm que carta jugare ahora?\n");
				print(cartas);
				printf("[109] Robar carta y saltar\n");
				printf("La carta en el pozo es:\n");
				print(poso);
				scanf("%d",&j);
				while(!(puedojugarla(cartas[j],col)))
				{
					if(j == 109)break;
					printf("Carta invalida, porfavor elegir otra carta\n");
					print(cartas);
					printf("La carta en el pozo es:\n");
					print(poso);
					scanf("%d",&j);
				}
				if(j != 109)
				{
					
					strcpy(mov, "mano2/");
					strcat(mov, cartas[j]);
	
					strcpy(poz, "pozo/");
					strcat(poz, poso[0]);
					eliminar_carta(poz);
					strcpy(poz, "pozo/");
					strcat(poz, cartas[j]);
					mover_carta(mov, poz);
	
					write(H1aP[1],cartas[j],strlen(cartas[j])+1);
				}

				else
				{
					robarXCartas(1,2);
					write(H1aP[1],"NONE",100);
				}

				liberarmemoria(cartas);
				liberarmemoria(poso);
            }
			else if(strcmp(pararecibir,"0") == 0){
				printf("Esperando\n");
				while((read(PaH1[0],pararecibir,100))<0);
				
				printf("Jugador 2: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
				printf("Impresionante\n");
				strcpy(CJTA,pararecibir);
			}
		}
		close(PaH1[0]);
		close(H1aP[1]);

	}
	else if(p == turnos[2]){
		char CJTA[11]; 
		//Cerrando escritura de padre a hijo
		close(PaH2[1]);

		//Cerrando lectura de hijo a padre
		close(H2aP[0]);

		//Cerrando cosas que no usara el proceso 1 saludos
		close(PaH1[0]);
		close(PaH3[0]);
		close(H1aP[1]);
		close(H3aP[1]);
		close(PaH1[1]);
		close(PaH3[1]);
		close(H1aP[0]);
		close(H3aP[0]);

		while(1){
			while((read(PaH2[0],pararecibir,100))<0);
			if(strcmp(pararecibir,"5")== 0) break;
			if(strcmp(pararecibir,"2") == 0){
				cartas = obtenercartas("mano3");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 3\n");
				printf("Jugador 3: Jugador 2 me jugo %s\n",CJTA);
				printf("Jugador 3: Mmmmmmm que carta jugare ahora?\n");
				print(cartas);

				printf("[109] Robar carta y saltar\n");

				printf("La carta en el pozo es:\n");
				print(poso);

				scanf("%d",&j);
				while(!(puedojugarla(cartas[j],col)))
				{
					if(j == 109)break;
					printf("Carta invalida, porfavor elegir otra carta\n");
					print(cartas);
					printf("La carta en el pozo es:\n");
					print(poso);
					scanf("%d",&j);
				}
				if(j != 109)	
					{
						strcpy(mov, "mano3/");
						strcat(mov, cartas[j]);

						strcpy(poz, "pozo/");
						strcat(poz, poso[0]);
						eliminar_carta(poz);
						printf("%s\n",poz);
						strcpy(poz, "pozo/");
						strcat(poz, cartas[j]);
						printf("%s\n",poz);
						mover_carta(mov, poz);
						write(H2aP[1],cartas[j],strlen(cartas[j])+1);
					}

				else
				{
					robarXCartas(1,3);
					write(H2aP[1],"NONE",100);
				}

				liberarmemoria(cartas);
				liberarmemoria(poso);
			}
			else if(strcmp(pararecibir,"1") == 0){
				while((read(PaH2[0],pararecibir,100))<0);
				printf("Jugador 3: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
				
				strcpy(CJTA,pararecibir);
			}
		}
		close(PaH2[0]);
		close(H2aP[1]);
	}
	else if(p == turnos[3]){
		char CJTA[11]; 
		//Cerrando escritura de padre a hijo
		close(PaH3[1]);

		//Cerrando lectura de hijo a padre
		close(H3aP[0]);

		//Cerrando cosas que no usara el proceso 1 saludos
		close(PaH1[0]);
		close(PaH2[0]);
		close(H1aP[1]);
		close(H2aP[1]);
		close(PaH1[1]);
		close(PaH2[1]);
		close(H1aP[0]);
		close(H2aP[0]);
		
		while(1){
			while((read(PaH3[0],pararecibir,100))<0);
			if(strcmp(pararecibir,"5")== 0) break;
			if(strcmp(pararecibir,"3") == 0){
				cartas=obtenercartas("mano4");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 2\n");
				printf("Jugador 4: Jugador 3 me jugo %s\n",CJTA);
				printf("Jugador 4: Mmmmmmm que carta jugare ahora?\n");
				print(cartas);

				printf("[109] Robar carta y saltar\n");

				printf("La carta en el pozo es:\n");
				print(poso);

				scanf("%d",&j);
				while(!(puedojugarla(cartas[j],col)))
				{
					if (j==109)break;
					printf("Carta invalida, porfavor elegir otra carta\n");
					print(cartas);
					printf("La carta en el pozo es:\n");
					print(poso);
					scanf("%d",&j);
				}
				if(j != 109)
				{
					strcpy(mov, "mano4/");
					strcat(mov, cartas[j]);

					strcpy(poz, "pozo/");
					strcat(poz, poso[0]);
					eliminar_carta(poz);
					strcpy(poz, "pozo/");
					strcat(poz, cartas[j]);
					mover_carta(mov, poz);
					write(H3aP[1],cartas[j],strlen(cartas[j])+1);
				}

				else
				{
					robarXCartas(1,4);
					write(H3aP[1],"NONE",100);
				}
				
				liberarmemoria(cartas);
				liberarmemoria(poso);
			}
			else if(strcmp(pararecibir,"2") == 0){
				while((read(PaH3[0],pararecibir,100))<0);
				printf("Jugador 4: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
				
				strcpy(CJTA,pararecibir);
			}	
		}
		close(PaH3[0]);
		close(H3aP[1]);
	}
	else printf("nandakorewa\n");
    
	free(PaH1);
	free(PaH2);
	free(PaH3);
	free(H1aP);
	free(H2aP);
	free(H3aP);
}
