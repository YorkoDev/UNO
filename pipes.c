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

int cantmano(int mano)
{
	DIR*d;
	struct dirent *dir;
	int i = 0;
	char a[2];
	char path[12];
	a[1] = '\0';
	strcpy(path,"./mano");
	sprintf(a, "%d", mano);
	strcat(path,a);
	d = opendir(path);

	if(d)
	{
		while((dir = readdir(d)) != NULL)
		{
			if(strcmp(".",dir->d_name) != 0 && strcmp("..",dir->d_name) != 0)
			{

				i++;
			}
		}
	}
	closedir(d);
	return i;
}
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

void mover_mazoapozo(char* carta, char* poso){
	char mov[30];
	char poz[30];
	strcpy(mov, "mazo/");
	strcat(mov, carta);

	strcpy(poz, "pozo/");
	strcat(poz, poso);
	eliminar_carta(poz);
	strcpy(poz, "pozo/");
	strcat(poz, carta);
	mover_carta(mov, poz);	
}

void mover_mazoamano(char* carta, int i){
	char mov[30];
	char poz[30];

	strcpy(mov, "mazo/");
	strcat(mov, carta);
	if(i == 1) strcpy(poz, "mano1/");
	else if(i == 2) strcpy(poz, "mano2/");
	else if(i == 3) strcpy(poz, "mano3/");
	else if(i == 4) strcpy(poz, "mano4/");
	strcat(poz, carta);
	mover_carta(mov, poz);
}

void mover_manoapozo(char* carta, char* poso,int i){
	char mov[30];
	char poz[30];

	if(i == 1) strcpy(mov, "mano1/");
	else if(i == 2) strcpy(mov, "mano2/");
	else if(i == 3) strcpy(mov, "mano3/");
	else if(i == 4) strcpy(mov, "mano4/");
	strcat(mov, carta);

	strcpy(poz, "pozo/");
	strcat(poz, poso);
	eliminar_carta(poz);
	strcpy(poz, "pozo/");
	strcat(poz, carta);
	mover_carta(mov, poz);
}

void jugando(){
	int p,i,j,k,direct=1,mas,boole=1,inic;
	char color;
	char paranegro[30];
    char* carta;
	char** cartas;
	char** poso;
	char col='z';
	int* H1aP;
	H1aP = (int*)malloc(sizeof(int)*2);
	int* H2aP;
	H2aP = (int*)malloc(sizeof(int)*2);
	int* H3aP;
	H3aP = (int*)malloc(sizeof(int)*2);
	int* PaH1;
	PaH1 = (int*)malloc(sizeof(int)*2);
	int* PaH2;
	PaH2 = (int*)malloc(sizeof(int)*2);
	int* PaH3;
	PaH3 = (int*)malloc(sizeof(int)*2);
	char paramandar[100];
	char pararecibir[100];
	
	pipe(H1aP); //H1 a padre
	pipe(H2aP); //H2 a padre
	pipe(H3aP); //H3 a padre
	pipe(PaH1); //padre a H1
	pipe(PaH2); //padre a H2
	pipe(PaH3); //padre a H3
	char turn[2];

	poso = obtenercartas("pozo");
	if(poso[0][0]== 'R')
	{
		direct = 0;
		printf("Se parte al revez\n"); 
	}
	liberarmemoria(poso);
	p = creacionprocesos();
	p = getpid();
	if(p == turnos[0]){
		char CJTA[18];
		strcpy(CJTA," ");
		i = 400;
		
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
			sprintf(turn,"%d",i%4);
			write(PaH1[1],turn,strlen(turn)+1);
			write(PaH2[1],turn,strlen(turn)+1);
			write(PaH3[1],turn,strlen(turn)+1);
			
			if(cantmano(1) == 1) printf("Al Jugador 1 le queda una carta\n");
			if(cantmano(2) == 1) printf("Al Jugador 2 le queda una carta\n");
			if(cantmano(3) == 1) printf("Al Jugador 3 le queda una carta\n");
			if(cantmano(4) == 1) printf("Al Jugador 4 le queda una carta\n");

			if(direct)
			{	
				if(i%4 == 0){
					cartas = obtenercartas("mano1");
					poso = obtenercartas("pozo");
					printf("\n");
					printf("Turno Jugador 1\n");
					if(strcmp(CJTA," ") != 0){
						if(strcmp(CJTA,"None") == 0) printf("Jugador 1: Cierto no jugo nada es mi oportunidad!\n");
						else if(strcmp(CJTA,"SALTA") == 0) printf("Jugador 1: Oh no soy saltado!\n");
						else if(col != 'z') printf("Jugador 1: Me cambiaron el color a %c\n",col);
						else if(pararecibir[0]== '+')
						{
							mas = (int)pararecibir[1] - 48;
							robarXCartas(mas, 2);
							printf("Jugador 1: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
							strcpy(pararecibir, "SALTA");
						}
					}

					if (strcmp(CJTA, " ") == 0)
					{
						if (poso[0][2] == 'N')
						{
							inic = rand()%4;
							if (inic == 0) col = 'R';
							else if (inic == 1) col = 'B';
							else if (inic == 2) col = 'G';
							else col = 'Y';

							if (poso[0][0] == '+')
							{
								printf("Jugador 1 parte recibiendo un +4 a la vena\n");
								printf("Se parte con el color: %c\n",col);
								strcpy(CJTA, "+4");
								robarXCartas(1,4);
							}
							else printf("Se parte con el color: %c\n",col);
						}
						else if(poso[0][0] == '+')
						{
							robarXCartas(2,1);
							printf("Jugador 1 parte recibiendo un +2 a la vena\n");
							strcpy(CJTA,"SALTA");
						}
						else if(poso[0][0] == 'S')
						{
							printf("Jugador 1 parte siendo skipeado (saltado)\n");
							strcpy(CJTA, "SALTA");
						}
					}

					if(strcmp(CJTA,"SALTA") != 0 && CJTA[1] != '4'){
						printf("Jugador 1: Mmmmmmm que carta jugare ahora?\n");
						printf("[109] Robar carta y saltar\n");
						k = print(cartas);
						printf("La carta en el pozo es:\n");
						print(poso);
						if(col != 'z') printf("El color actual es %c\n", col);
						scanf("%d",&j);

						while(k < j && !(puedojugarla(cartas[j],col)))
						{
							printf("Carta invalida, porfavor elegir otra carta\n");
							k = print(cartas);
							printf("La carta en el pozo es:\n");
							print(poso);
							if(col != 'z') printf("El color actual es %c\n", col);
							scanf("%d",&j);

						}
						if (j != 109)
						{					
							mover_manoapozo(cartas[j],poso[0],1);
							if (cantmano(1) == 0)
							{
								printf("Jugador 1 GANO :DDDDDD\n");
								write(PaH2[1],"F",2);
								write(PaH1[1],"F",2);
								write(PaH3[1],"F",2);
								break;
							}
							else if(cartas[j][0] == 'R')
							{
								direct = 0;
								strcpy(turn,"7");
								write(PaH1[1],turn,strlen(turn)+1);
								write(PaH2[1],turn,strlen(turn)+1);
								write(PaH3[1],turn,strlen(turn)+1);
							}
							else if(cartas[j][0] == 'S') write(PaH1[1],"SALTA",6);
							else if(cartas[j][2] == 'N'){
								
								printf("Jugador 1: MMMMM que color eligire?????\n");
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
								while(k > 4){
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
								}
								char boi[1];
								if(k == 1) color = 'R';
								else if(k == 2) color = 'G';
								else if(k == 3) color = 'B';
								else color = 'Y';
								
								boi[0] = color;

								strcpy(paranegro,cartas[j]);
								strcat(paranegro,boi);
								
								write(PaH1[1],paranegro,strlen(paranegro)+1);
								//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
								

							}
							else write(PaH1[1],cartas[j],strlen(cartas[j])+1);
						}
						else
						{
							carta = cartaMazo();
							
							if(puedojugarla(carta,col)){
								printf("Jugador 1: Wenaaa la buena suerte esta de mi lado %s\n",carta);
								mover_mazoapozo(carta,poso[0]);
								if(carta[0] == 'S'){
									char Salta[7];
									strcpy(Salta,"SALTA");
									write(PaH1[1],Salta,7);
								}
								else if(carta[0] == 'R')
								{
									direct = 0;
									strcpy(turn,"7");
									write(PaH1[1],turn,strlen(turn)+1);
									write(PaH2[1],turn,strlen(turn)+1);
									write(PaH3[1],turn,strlen(turn)+1);
								}
								else if(carta[2] == 'N'){
									printf("Jugador 1: MMMMM que color eligire?????\n");
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
									while(k > 4){
										printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
										scanf("%d",&k);
									}
									char boi[1];
									if(k == 1) color = 'R';
									else if(k == 2) color = 'G';
									else if(k == 3) color = 'B';
									else color = 'Y';
									
									boi[0] = color;

									strcpy(paranegro,carta);
									strcat(paranegro,boi);
									
									write(PaH1[1],paranegro,strlen(paranegro)+1);
									//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
									

								}
								else write(PaH1[1],carta,strlen(carta)+1);					
							}
							else if(poso[0][2]== 'N'){
								mover_mazoamano(carta,1);
								char pal[30];
								strcpy(pal,"X");
								pal[1] = col;
								printf("%s\n",pal);
								write(PaH1[1],pal, strlen(pal)+1);
							}
							else
							{
								printf("Jugador 1: Ay no que mala pata! Robe %s\n",carta);
								
								mover_mazoamano(carta,1);
								char None[5];
								strcpy(None,"None");
								write(PaH1[1],None,5);
							}
							free(carta);
						}
					}
					else if(CJTA[1] == '4'){

						char pal[30];
						strcpy(pal,"X");
						pal[1] = CJTA[8];
						printf("%s\n",pal);
						write(PaH1[1],pal, strlen(pal)+1);
					}
					else if(strcmp(CJTA,"SALTA") == 0) write(PaH1[1],"None",8);
					liberarmemoria(cartas);
					liberarmemoria(poso);
					
					
				}
				else if (i%4 == 1){
					while((read(H1aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: Te salvaste jugador 3 el jugador 2 no jugo nada xd\n");
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 0;
						write(PaH2[1],pararecibir,strlen(pararecibir)+1);
						write(PaH3[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;
						
					}
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 2 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 3 TE VAN A SALTAR!\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[2] == 'N') printf("Jugador 1: Uuuuh cambiaron el color a %c\n",pararecibir[8]);
					else if(pararecibir[0] == 'X') printf("Jugador 1: jijijijijjij\n");
					else if(pararecibir[0] == 'F')
					{
						printf("Jugador 2 GANO :DDDDDD\n");
						write(PaH2[1],"F",2);
						write(PaH3[1],"F",2);
						break;
					}
					else printf("Jugador 1: Asi que el Jugador 2 puso un %s... Suerte con eso Jugador 3\n",pararecibir);

					if(boole == 1) write(PaH2[1],pararecibir,strlen(pararecibir)+1);
				}
				else if (i%4 == 2){
					while((read(H2aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: Te salvaste Jugador 4 el jugador 3 no jugo nada xd\n");
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 0;
						write(PaH1[1],pararecibir,strlen(pararecibir)+1);
						write(PaH3[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;
						
					}
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 3 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 4 TE VAN A SALTAR!\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[1] == '4') printf("Jugador 1: Uuuh te van a hacer robar 4\n");
					else if(pararecibir[2] == 'N') printf("Jugador 1: Uuuuh cambiaron el color a %c\n",pararecibir[8]);
					else if(pararecibir[0] == 'X') printf("Jugador 1: jijijijijjij\n");
					else if(pararecibir[0] == 'F')
					{
						printf("Jugador 3 GANO :DDDDDD\n");
						write(PaH1[1],"F",2);
						write(PaH3[1],"F",2);
						break;
					}
					else printf("Jugador 1: Asi que el Jugador 3 puso un %s... Suerte con eso Jugador 4\n",pararecibir);
					
					if(boole == 1)write(PaH3[1],pararecibir,strlen(pararecibir)+1);
				}
				else if (i%4 == 3){
					while((read(H3aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: ME SALVE!!!!\n");
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 4 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 0;
						write(PaH1[1],pararecibir,strlen(pararecibir)+1);
						write(PaH2[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;	
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 1 TE VAN A SALTAR! Oh wait...\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];
						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 1: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 1: Me comi el sendo +4\n");
							robarXCartas(4,1);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 1);
						printf("Jugador 1: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else if(pararecibir[0] == 'F')
					{
						printf("Jugador 4 GANO :DDDDDD\n");
						write(PaH1[1],"F",2);
						write(PaH2[1],"F",2);
						break;
					}
					else printf("Jugador 1: Asi que el Jugador 4 puso un %s... Suerte con eso Jugador 1, oh cresta soy yo\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}
			}
			else
			{
				if(i%4 == 0){
					cartas = obtenercartas("mano1");
					poso = obtenercartas("pozo");
					printf("\n");
					printf("Turno Jugador 1\n");
					if(strcmp(CJTA," ") != 0){
						if(strcmp(CJTA,"None") == 0) printf("Jugador 1: Cierto no jugo nada es mi oportunidad!\n");
						else if(strcmp(CJTA,"SALTA") == 0) printf("Jugador 1: Oh no soy saltado!\n");
						else if(col != 'z') printf("Jugador 1: Me cambiaron el color a %c\n",col);
						else if(pararecibir[0]== '+')
						{
							mas = (int)pararecibir[1] - 48;
							robarXCartas(mas, 2);
							printf("Jugador 1: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
							strcpy(pararecibir, "SALTA");
						}

					}

					if (strcmp(CJTA, " ") == 0)
					{
						if (poso[0][2] == 'N')
						{
							inic = rand()%4;
							if (inic == 0) col = 'R';
							else if (inic == 1) col = 'B';
							else if (inic == 2) col = 'G';
							else col = 'Y';

							if (poso[0][0] == '+')
							{
								strcpy(CJTA, "+4");
								robarXCartas(1,4);
							}
						}
						else if(poso[0][0] == '+')
						{
							robarXCartas(2,1);
							strcpy(CJTA,"SALTA");
						}
						else if(poso[0][0] == 'S')
						{
							strcpy(CJTA, "SALTA");
						}
						else if(poso[0][0] == 'R')
						{
							strcpy(CJTA, "7");
						}
					}

					if(strcmp(CJTA,"SALTA") != 0 && CJTA[1] != '4'){
						printf("Jugador 1: Mmmmmmm que carta jugare ahora?\n");
						printf("[109] Robar carta y saltar\n");
						k = print(cartas);
						printf("La carta en el pozo es:\n");
						print(poso);
						if(col != 'z') printf("El color actual es %c\n", col);
						scanf("%d",&j);

						while(k < j && !(puedojugarla(cartas[j],col)))
						{
							printf("Carta invalida, porfavor elegir otra carta\n");
							k = print(cartas);
							printf("La carta en el pozo es:\n");
							print(poso);
							if(col != 'z') printf("El color actual es %c\n", col);
							scanf("%d",&j);
						}
						if (j != 109)
						{					
							mover_manoapozo(cartas[j],poso[0],1);
							if (cantmano(1) == 0)
							{
								printf("Jugador 1 GANO :DDDDDD\n");
								write(PaH2[1],"F",2);
								write(PaH1[1],"F",2);
								write(PaH3[1],"F",2);
								break;
							}
							if(cartas[j][0] == 'S') write(PaH3[1],"SALTA",6);
							if(cartas[j][0] == 'R')
							{
								direct = 1;
								strcpy(turn,"7");
								write(PaH1[1],turn,strlen(turn)+1);
								write(PaH2[1],turn,strlen(turn)+1);
								write(PaH3[1],turn,strlen(turn)+1);
							}
							else if(cartas[j][2] == 'N'){
								
								printf("Jugador 1: MMMMM que color eligire?????\n");
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
								while(k > 4){
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
								}
								char boi[1];
								if(k == 1) color = 'R';
								else if(k == 2) color = 'G';
								else if(k == 3) color = 'B';
								else color = 'Y';
								
								boi[0] = color;

								strcpy(paranegro,cartas[j]);
								strcat(paranegro,boi);
								
								write(PaH3[1],paranegro,strlen(paranegro)+1);
							}
							else write(PaH3[1],cartas[j],strlen(cartas[j])+1);
						}
						else
						{
							carta = cartaMazo();
							
							if(puedojugarla(carta,col)){
								printf("Jugador 1: Wenaaa la buena suerte esta de mi lado %s\n",carta);
								mover_mazoapozo(carta,poso[0]);
								if(carta[0] == 'S'){
									char Salta[7];
									strcpy(Salta,"SALTA");
									write(PaH3[1],Salta,7);
								}
								else if(carta[0] == 'R')
								{
									direct = 1;
									strcpy(turn,"7");
									write(PaH1[1],turn,strlen(turn)+1);
									write(PaH2[1],turn,strlen(turn)+1);
									write(PaH3[1],turn,strlen(turn)+1);
								}
								else if(carta[2] == 'N'){
									printf("Jugador 1: MMMMM que color eligire?????\n");
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
									while(k > 4){
										printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
										scanf("%d",&k);
									}
									char boi[1];
									if(k == 1) color = 'R';
									else if(k == 2) color = 'G';
									else if(k == 3) color = 'B';
									else color = 'Y';
									
									boi[0] = color;

									strcpy(paranegro,carta);
									strcat(paranegro,boi);
									
									write(PaH3[1],paranegro,strlen(paranegro)+1);
								}
								else write(PaH3[1],carta,strlen(carta)+1);					
							}
							else if(poso[0][2]== 'N'){
								mover_mazoamano(carta,1);
								char pal[30];
								strcpy(pal,"X");
								pal[1] = col;
								printf("%s\n",pal);
								write(PaH3[1],pal, strlen(pal)+1);
							}
							else{
								printf("Jugador 1: Ay no que mala pata! Robe %s\n",carta);
								
								mover_mazoamano(carta,1);
								char None[5];
								strcpy(None,"None");
								write(PaH3[1],None,5);
							}
							free(carta);
						}
					}
					else if(CJTA[1] == '4'){

						char pal[30];
						strcpy(pal,"X");
						pal[1] = CJTA[8];
						write(PaH3[1],pal, strlen(pal)+1);
					}
					else if(strcmp(CJTA,"SALTA") == 0) write(PaH3[1],"None",8);
					liberarmemoria(cartas);
					liberarmemoria(poso);
					
					
				}
				else if (i%4 == 1)
				{
					while((read(H1aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: ME SALVE!!!!\n");
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 1;
						write(PaH2[1],pararecibir,strlen(pararecibir)+1);
						write(PaH3[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;
						
					}
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 2 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 1 TE VAN A SALTAR! Oh wait...\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];
						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 1: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 1: Me comi el sendo +4\n");
							robarXCartas(4,1);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 1);
						printf("Jugador 1: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 1: Asi que el Jugador 2 puso un %s... Suerte con eso Jugador 1, oh cresta soy yo\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}
				else if (i%4 == 2)
				{
					while((read(H2aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: Te salvaste Jugador 2 el jugador 3 no jugo nada xd\n");
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 1;
						write(PaH1[1],pararecibir,strlen(pararecibir)+1);
						write(PaH3[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;
						
					}
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 3 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 2 TE VAN A SALTAR!\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[1] == '4') printf("Jugador 1: Uuuh te van a hacer robar 4\n");
					else if(pararecibir[2] == 'N') printf("Jugador 1: Uuuuh cambiaron el color a %c\n",pararecibir[8]);
					else if(pararecibir[0] == 'X') printf("Jugador 1: jijijijjijij\n");
					else printf("Jugador 1: Asi que el Jugador 3 puso un %s... Suerte con eso Jugador 2\n",pararecibir);
					write(PaH1[1],pararecibir,strlen(pararecibir)+1);
				}
				else if (i%4 == 3)
				{
					while((read(H3aP[0],pararecibir,100))<0);
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 1: Te salvaste jugador 3 el jugador 4 no jugo nada xd\n");
					else if(strcmp(pararecibir,"7") == 0){
						printf("Jugador 1: CAMBIARON EL SENTIDO DEL JUEGO!\n");
						direct = 1;
						write(PaH1[1],pararecibir,strlen(pararecibir)+1);
						write(PaH2[1],pararecibir,strlen(pararecibir)+1);
						boole = 0;	
					}
					else if(strcmp(pararecibir,"SALTADO") == 0){
						printf("Jugador 1: El jugador 4 fue saltado!\n");
						strcpy(pararecibir,"None");
					}
					else if(strcmp(pararecibir,"SALTA") == 0){
						printf("Jugador 1: OH NO JUGADOR 3 TE VAN A SALTAR!\n");
						strcpy(pararecibir,"SALTA");
					}
					else if(pararecibir[2] == 'N') printf("Jugador 1: Uuuuh cambiaron el color a %c\n",pararecibir[8]);
					else if(pararecibir[0] == 'X') printf("Jugador 1: jijijijjijij\n");
					else printf("Jugador 1: Asi que el Jugador 4 puso un %s... Suerte con eso Jugador 3\n",pararecibir);

					write(PaH2[1],pararecibir,strlen(pararecibir)+1);
				}
			}
			
			if(direct == 1) i++;
			else i--;
			col = 'z';
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
		char CJTA[18]; //Carta jugada turno anterior
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
			if(strcmp(pararecibir,"5")== 0 || pararecibir[0] == 'F') break;
			if(strcmp(pararecibir,"7") == 0 && direct == 1){
				printf("Jugador 2: ODITNES LE NORAIBMAC\n"); 
				direct = 0;
			}
			else if(strcmp(pararecibir,"7") == 0 && direct == 0){
				printf("Jugador 2: ODITNES LE NORAIBMAC\n"); 
				direct = 1;
			}
			if(strcmp(pararecibir,"1") == 0){
                cartas=obtenercartas("mano2");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 2\n");
				if(strcmp(CJTA,"None") == 0) printf("Jugador 2: Cierto no jugo nada es mi oportunidad!\n");
				else if(strcmp(CJTA,"SALTA") == 0) printf("Jugador 2: Oh no soy saltado!\n");
				else if(col != 'z') printf("Jugador 2: Me cambiaron el color a %c\n",col);

				
				
				if(strcmp(CJTA,"SALTA") != 0 && CJTA[1] != '4'){
					printf("Jugador 2: Mmmmmmm que carta jugare ahora?\n");
					k = print(cartas);
					printf("[109] Robar carta y saltar\n");
					printf("La carta en el pozo es:\n");
					print(poso);
					if(col != 'z') printf("El color actual es %c\n", col);
					scanf("%d",&j);
					while(k < j && !(puedojugarla(cartas[j],col)))
					{
						printf("Carta invalida, porfavor elegir otra carta\n");
						k = print(cartas);
						printf("La carta en el pozo es:\n");
						print(poso);
						if(col != 'z') printf("El color actual es %c\n", col);
						scanf("%d",&j);
					}
					if(j != 109)
					{
						
						mover_manoapozo(cartas[j],poso[0],2);
						if (cantmano(2) == 0)
							{
								printf("Jugador 2 GANO :DDDDDD\n");
								write(H1aP[1],"F",2);
								break;
							}
						if(cartas[j][0] == 'R')
						{
							if (direct == 0)direct = 1;
							else direct = 0;
							strcpy(turn,"7");
							write(H1aP[1],turn,strlen(turn)+1);
						}
						else if(cartas[j][0] == 'S'){
							write(H1aP[1],"SALTA",6);
						}
						else if(cartas[j][2] == 'N'){
							
							printf("Jugador 2: MMMMM que color eligire?????\n");
							printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
							scanf("%d",&k);
							while(k > 4){
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
							}
							char boi[1];
							if(k == 1) color = 'R';
							else if(k == 2) color = 'G';
							else if(k == 3) color = 'B';
							else color = 'Y';
							
							boi[0] = color;

							strcpy(paranegro,cartas[j]);
							strcat(paranegro,boi);
							
							write(H1aP[1],paranegro,strlen(paranegro)+1);
							//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
							

						}
						else write(H1aP[1],cartas[j],strlen(cartas[j])+1);
						
					}
					else
					{
						carta = cartaMazo();
						
						if(puedojugarla(carta,col)){
							printf("Jugador 2: Wenaaa la buena suerte esta de mi lado %s\n",carta);
							mover_mazoapozo(carta,poso[0]);	
							if(carta[0] == 'S'){
								write(H1aP[1],"SALTA",6);
							}
							else if(carta[0] == 'R')
							{
								if (direct == 0)direct = 1;
								else direct = 0;
								strcpy(turn,"7");
								write(H1aP[1],turn,strlen(turn)+1);
							}
							else if(carta[2] == 'N'){
								printf("Jugador 2: MMMMM que color eligire?????\n");
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
								while(k > 4){
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
								}
								char boi[1];
								if(k == 1) color = 'R';
								else if(k == 2) color = 'G';
								else if(k == 3) color = 'B';
								else color = 'Y';
								
								boi[0] = color;

								strcpy(paranegro,carta);
								strcat(paranegro,boi);
								
								write(H1aP[1],paranegro,strlen(paranegro)+1);
								//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
								

							}
							else write(H1aP[1],carta,strlen(carta)+1);					
						}
						else if(poso[0][2]== 'N'){
								mover_mazoamano(carta,2);
								char pal[30];
								strcpy(pal,"X");
								pal[1] = col;
								printf("%s\n",pal);
								write(H1aP[1],pal, strlen(pal)+1);
							}
	
						else{
							printf("Jugador 2: Ay no que mala pata! Robe %s\n",carta);
							mover_mazoamano(carta,2);

							char None[5];
							strcpy(None,"None");
							printf("None %s\n",None);
							write(H1aP[1],None,5);
						}
						free(carta);
					}
				}
				else if(CJTA[1] == '4'){
					char pal[30];
					strcpy(pal,"X");
					pal[1] = CJTA[8];
					write(H1aP[1],pal, strlen(pal)+1);
				}
				else if(strcmp(CJTA,"SALTA") == 0) write(H1aP[1],"SALTADO",8);
				liberarmemoria(cartas);
				liberarmemoria(poso);
				col = 'z';
            }
			else if(strcmp(pararecibir,"0") == 0 && direct == 1)//Turno anterior al jugador 2
			{ 
				while((read(PaH1[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(strcmp(pararecibir,"7") == 0){
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 2: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 2: Me saltaron F :c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];
						
						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 2: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 2: Me comi el sendo +4\n");
							robarXCartas(4,2);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 2);
						printf("Jugador 2: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 2: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 2: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 0;
				}
			}
			else if(strcmp(pararecibir,"2") == 0 && direct == 0)//Turno anterior al jugador 2
			{ 
				while((read(PaH1[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(pararecibir[0] != '7'){
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 2: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 2: Me saltaron F :c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];

						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 2: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 2: Me comi el sendo +4\n");
							robarXCartas(4,2);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 2);
						printf("Jugador 2: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 2: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 2: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 1;
				}
			}
		}
		close(PaH1[0]);
		close(H1aP[1]);

	}
	else if(p == turnos[2]){
		char CJTA[18]; 
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
			if(strcmp(pararecibir,"5")== 0 || pararecibir[0] == 'F') break;
			if(strcmp(pararecibir,"7") == 0 && direct == 1){
				printf("Jugador 3: ODITNES LE NORAIBMAC\n"); 
				direct = 0;
			}
			else if(strcmp(pararecibir,"7") == 0 && direct == 0){
				printf("Jugador 3: ODITNES LE NORAIBMAC\n"); 
				direct = 1;
			}
			if(strcmp(pararecibir,"2") == 0){
				cartas = obtenercartas("mano3");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 3\n");
				if(strcmp(CJTA,"None") == 0) printf("Jugador 3: Cierto no jugo nada es mi oportunidad!\n");
				else if(strcmp(CJTA,"SALTA") == 0) printf("Jugador 3: Oh no soy saltado!\n");

				
				if(strcmp(CJTA,"SALTA") != 0 && CJTA[1] != '4'){
					printf("Jugador 3: Mmmmmmm que carta jugare ahora?\n");
					k = print(cartas);

					printf("[109] Robar carta y saltar\n");

					printf("La carta en el pozo es:\n");
					print(poso);
					if(col != 'z') printf("El color actual es %c\n", col);
					scanf("%d",&j);
					while(k < j && !(puedojugarla(cartas[j],col)))
					{
						printf("Carta invalida, porfavor elegir otra carta\n");
						k = print(cartas);
						printf("La carta en el pozo es:\n");
						print(poso);
						if(col != 'z') printf("El color actual es %c\n", col);
						scanf("%d",&j);
					}
					if(j != 109)	
					{
						mover_manoapozo(cartas[j],poso[0],3);
						if (cantmano(3) == 0)
						{
							printf("Jugador 3 GANO :DDDDDD\n");
							write(H2aP[1],"F",2);
							break;
						}
						if(cartas[j][0] == 'S'){
							write(H2aP[1],"SALTA",6);
						}
						else if(cartas[j][0] == 'R')
						{
							if (direct == 0)direct = 1;
							else direct = 0;
							strcpy(turn,"7");
							write(H2aP[1],turn,strlen(turn)+1);
						}
						else if(cartas[j][2] == 'N'){
							
							printf("Jugador 3: MMMMM que color eligire?????\n");
							printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
							scanf("%d",&k);
							while(k > 4){
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
							}
							char boi[1];
							if(k == 1) color = 'R';
							else if(k == 2) color = 'G';
							else if(k == 3) color = 'B';
							else color = 'Y';
							
							boi[0] = color;

							strcpy(paranegro,cartas[j]);
							strcat(paranegro,boi);
							
							write(H2aP[1],paranegro,strlen(paranegro)+1);
							//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
							
						}
						else write(H2aP[1],cartas[j],strlen(cartas[j])+1);
					}
					else
					{
						carta = cartaMazo();
						
						if(puedojugarla(carta,col)){
							printf("Jugador 3: Wenaaa la buena suerte esta de mi lado %s\n",carta);
							mover_mazoapozo(carta,poso[0]);
							if(carta[0] == 'S'){
								write(H2aP[1],"SALTA",6);
							}
							else if(carta[0] == 'R')
							{
								if (direct == 0)direct = 1;
								else direct = 0;
								strcpy(turn,"7");
								write(H2aP[1],turn,strlen(turn)+1);
							}
							else if(carta[2] == 'N'){
								printf("Jugador 3: MMMMM que color eligire?????\n");
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
								while(k > 4){
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
								}
								char boi[1];
								if(k == 1) color = 'R';
								else if(k == 2) color = 'G';
								else if(k == 3) color = 'B';
								else color = 'Y';
								
								boi[0] = color;

								strcpy(paranegro,carta);
								strcat(paranegro,boi);
								
								write(H2aP[1],paranegro,strlen(paranegro)+1);
								//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
								

							}
							else write(H2aP[1],carta,strlen(carta)+1);	
											
						}
						else if(poso[0][2]== 'N'){
								mover_mazoamano(carta,1);
								char pal[30];
								strcpy(pal,"X");
								pal[1] = col;
								printf("%s\n",pal);
								write(H2aP[1],pal, strlen(pal)+1);
							}
						else{
							printf("Jugador 3: Ay no que mala pata! Robe %s\n",carta);
							mover_mazoamano(carta,3);

							char None[5];
							strcpy(None,"None");
							printf("None %s\n",None);
							write(H2aP[1],None,5);
						}
						free(carta);
					}
				}
				else if(CJTA[1] == '4'){
					char pal[30];
					strcpy(pal,"X");
					pal[1] = CJTA[8];
					write(H2aP[1],pal, strlen(pal)+1);
				}
				else if(strcmp(CJTA,"SALTA") == 0) write(H2aP[1],"SALTADO",8);
				liberarmemoria(cartas);
				liberarmemoria(poso);
				col = 'z';
			}
			else if(strcmp(pararecibir,"1") == 0 && direct == 1){
				while((read(PaH2[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(strcmp(pararecibir,"7") != 0){
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 3: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 3: Me saltaron F :'c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];

						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 3: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 3: Me comi el sendo +4\n");
							robarXCartas(4,3);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 3);
						printf("Jugador 3: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 3: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 3: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 0;
				}
			}
			else if(strcmp(pararecibir,"3") == 0 && direct == 0){
				while((read(PaH2[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(strcmp(pararecibir,"7") != 0){	
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 3: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 3: Me saltaron F :'c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];

						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 3: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 3: Me comi el sendo +4\n");
							robarXCartas(4,3);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 3);
						printf("Jugador 3: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 3: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 3: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 1;
				}
			}
		}
		close(PaH2[0]);
		close(H2aP[1]);
	}
	else if(p == turnos[3]){
		char CJTA[18]; 
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
			if(strcmp(pararecibir,"5")== 0 || pararecibir[0] == 'F') break;
			if(strcmp(pararecibir,"7") == 0 && direct == 1){
				printf("Jugador 4: ODITNES LE NORAIBMAC\n"); 
				direct = 0;
			}
			else if(strcmp(pararecibir,"7") == 0 && direct == 0){
				printf("Jugador 4: ODITNES LE NORAIBMAC\n"); 
				direct = 1;
			}
			if(strcmp(pararecibir,"3") == 0){
				cartas=obtenercartas("mano4");
				poso = obtenercartas("pozo");
				printf("\n");
				printf("Turno Jugador 4\n");
				if(strcmp(CJTA,"None") == 0) printf("Jugador 4: Cierto no jugo nada es mi oportunidad!\n");


				if(strcmp(CJTA,"SALTA") != 0 && CJTA[1] != '4'){
					printf("Jugador 4: Mmmmmmm que carta jugare ahora?\n");
					k = print(cartas);

					printf("[109] Robar carta y saltar\n");

					printf("La carta en el pozo es:\n");
					print(poso);
					if(col != 'z') printf("El color actual es %c\n", col);
					scanf("%d",&j);
					while(k < j && !(puedojugarla(cartas[j],col)))
					{
						if (j==109)break;
						printf("Carta invalida, porfavor elegir otra carta\n");
						k = print(cartas);
						printf("La carta en el pozo es:\n");
						print(poso);
						if(col != 'z') printf("El color actual es %c\n", col);
						scanf("%d",&j);
					}
					if(j != 109)
					{
						mover_manoapozo(cartas[j],poso[0],4);
						if (cantmano(4) == 0)
						{
							printf("Jugador 4 GANO :DDDDDD\n");
							write(H2aP[1],"F",2);
							break;
						}
						if(cartas[j][0] == 'S'){
							write(H3aP[1],"SALTA",6);
						}
						else if(cartas[j][0]== 'R')
						{
							if (direct == 0)direct = 1;
							else direct = 0;
							strcpy(turn,"7");
							write(H3aP[1],turn,strlen(turn)+1);
						}
						else if(cartas[j][2] == 'N'){
							
							printf("Jugador 4: MMMMM que color eligire?????\n");
							printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
							scanf("%d",&k);
							while(k > 4){
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
							}
							char boi[1];
							if(k == 1) color = 'R';
							else if(k == 2) color = 'G';
							else if(k == 3) color = 'B';
							else color = 'Y';
							
							boi[0] = color;

							strcpy(paranegro,cartas[j]);
							strcat(paranegro,boi);
							
							write(H3aP[1],paranegro,strlen(paranegro)+1);
							
						}
						else write(H3aP[1],cartas[j],strlen(cartas[j])+1);
					}

					else
					{
						carta = cartaMazo();
						
						if(puedojugarla(carta,col))
						{
							printf("Jugador 4: Wenaaa la buena suerte esta de mi lado %s\n",carta);
							mover_mazoapozo(carta,poso[0]);	
							if(carta[0] == 'S'){
								write(H3aP[1],"SALTA",6);
							}
							else if(carta[0]== 'R')
							{
								if (direct == 0)direct = 1;
								else direct = 0;
								strcpy(turn,"7");
								write(H3aP[1],turn,strlen(turn)+1);
							}
							else if(carta[2] == 'N'){
								printf("Jugador 4: MMMMM que color eligire?????\n");
								printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
								scanf("%d",&k);
								while(k > 4){
									printf("[1] Rojo\n[2] Verde\n[3] Azul\n[4] Amarillo\n");
									scanf("%d",&k);
								}
								char boi[1];
								if(k == 1) color = 'R';
								else if(k == 2) color = 'G';
								else if(k == 3) color = 'B';
								else color = 'Y';
								
								boi[0] = color;

								strcpy(paranegro,carta);
								strcat(paranegro,boi);
								
								write(H3aP[1],paranegro,strlen(paranegro)+1);
								//SI POR AHORA LO QUE HICEE FUE EL JUGADOR 1 PUDIERA ELEGIR EL COLOR SOLO SI LA CARTA ES NEGRA
								

							}
							else write(H3aP[1],carta,strlen(carta)+1);						
						}
						else if(poso[0][2]== 'N')
						{
							mover_mazoamano(carta,1);
							char pal[30];
							strcpy(pal,"X");
							pal[1] = col;
							printf("%s\n",pal);
							write(H3aP[1],pal, strlen(pal)+1);
						}
						else{
							printf("Jugador 4: Ay no que mala pata! Robe %s\n",carta);
							mover_mazoamano(carta,4);
							char None[5];
							strcpy(None,"None");
							printf("None %s\n",None);
							write(H3aP[1],None,5);
						}
						free(carta);
					}
				}
				else if(CJTA[1] == '4'){
					char pal[30];
					strcpy(pal,"X");
					pal[1] = CJTA[8];
					write(H3aP[1],pal, strlen(pal)+1);
				}
				else if(strcmp(CJTA,"SALTA") == 0) write(H3aP[1],"SALTADO",8);
				liberarmemoria(cartas);
				liberarmemoria(poso);
				col = 'z';
			}
			else if(strcmp(pararecibir,"2") == 0 && direct == 1){
				while((read(PaH3[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(strcmp(pararecibir,"7") != 0){
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 4: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 4: Me saltaron F :'c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];

						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 4: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 4: Me comi el sendo +4\n");
							robarXCartas(4,4);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 4);
						printf("Jugador 4: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 4: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 4: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 0;
				}
			}
			else if(strcmp(pararecibir,"0") == 0 && direct == 0){
				while((read(PaH3[0],pararecibir,100))<0);
				if(pararecibir[0] == 'F') break;
				else if(strcmp(pararecibir,"7") != 0){
					if(strcmp(pararecibir,"None") == 0) printf("Jugador 4: NO JUGO NADA VIVA CHILE\n");
					else if(strcmp(pararecibir,"SALTA") == 0) printf("Jugador 4: Me saltaron F :'c\n");
					else if(pararecibir[0] == 'X'){
						col = pararecibir[1];

						strcpy(pararecibir,"None");
					}
					else if(pararecibir[2] == 'N'){
						if(pararecibir[0] == 'C'){
							col = pararecibir[8];
							printf("Jugador 4: Oh cambiaron el color!\n");
							pararecibir[8] = '\0';
						}
						else
						{
							printf("Jugador 4: Me comi el sendo +4\n");
							robarXCartas(4,4);
						}
					}
					else if(pararecibir[0]== '+')
					{
						mas = (int)pararecibir[1] - 48;
						robarXCartas(mas, 4);
						printf("Jugador 4: Me tiran un +%d y ademas no puedo jugar mi turno :c\n", mas);
						strcpy(pararecibir, "SALTA");
					}
					else printf("Jugador 4: Nani!!!! Asi que jugaste esa carta %s\n",pararecibir);
					strcpy(CJTA,pararecibir);
				}else{
					printf("Jugador 4: CAMBIARON EL SENTIDO DEL JUEGO AAAAA \n");
					direct = 1;
				}
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