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


int main(){
    int PH[2]; //pipe padre a hijo
    int HP[2]; //hijo a padre

    char fixed_str[] = "ayuda";
    char input_str[100];

    int p,i;

    if(pipe(PH) == -1){
        printf("Creo que algo salio mal\n");
        exit(1);
    }
    if(pipe(HP) == -1){
        printf("Creo que algo salio mal\n");
        exit(1);
    }

    scanf("%s",input_str);
    p = fork();

    if(p < 0){
        printf("Fork fallo\n");
    }
    else if(p > 0){
        char concat_str[100];

        close(PH[0]);

        write(PH[1],input_str,strlen(input_str)+1);
        close(PH[1]);

        wait(NULL);

        close(HP[1]);

        read(HP[0],concat_str,100);
        printf("Concatenated string %s\n",concat_str);
        close(HP[0]);
    }
    else{
        close(PH[1]);

        char concat_str[100];
        read(PH[0],concat_str,100);

        int k = strlen(concat_str);
        int i;
        for(i = 0; i < strlen(fixed_str);i++) concat_str[k++] = fixed_str[i];
        concat_str[k] = '\0';
        
        close(HP[0]);
        close(PH[0]);

        write(HP[1],concat_str,strlen(concat_str)+1);
        close(HP[1]);
        exit(1);
    }
    
	int H1aP[2];
	int H2aP[2];
	int H3aP[2];
	int PaH1[2];
	int PaH2[2];
	int PaH3[2];
	char paramandar[100];
	char pararecibir[100];
	pipe(H1aP); //H1 a padre
	pipe(H2aP); //H2 a padre
	pipe(H3aP); //H3 a padre
	pipe(PaH1); //padre a H1
	pipe(PaH2); //padre a H2
	pipe(PaH3); //padre a H3
	
	p = creacionprocesos();
    printf("[");
    for(p = 0; p < 4-1; p++) printf("%d,",turnos[p]);
    printf("%d]\n",turnos[p]);

	p = getpid();
	if(p == turnos[0]){
		
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
		printf("Seguir jugando?\n");
		printf("[1] Si\n");
		printf("[2] No\n");
		scanf("%s",paramandar);
		while(strcmp(paramandar,"1")==0){
			strcpy(turn,"");
			sprintf(turn,"%d",i);
			write(PaH1[1],turn,strlen(turn)+1);
			write(PaH2[1],turn,strlen(turn)+1);
			write(PaH3[1],turn,strlen(turn)+1);
			if(i == 0) printf("Es el turno del papa\n");
			
			i = (i + 1)%4;
			printf("Seguir jugando?\n");
			printf("[1] Si\n");
			printf("[2] No\n");
			scanf("%s",paramandar);
		}
		strcpy(turn,"");
		sprintf(turn,"%d",5);
		write(PaH1[1],turn,strlen(turn)+1);
		write(PaH2[1],turn,strlen(turn)+1);
		write(PaH3[1],turn,strlen(turn)+1);


	}
	else if(p == turnos[1]){
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
			if(strcmp(pararecibir,"1") == 0) printf("Es el turno del hijo 1\n");
		}
		close(PaH1[0]);
		close(H1aP[1]);

	}
	else if(p == turnos[2]){
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
			if(strcmp(pararecibir,"2") == 0) printf("Es el turno del hijo 2\n");
		}
		close(PaH2[0]);
		close(H2aP[1]);
	}
	else if(p == turnos[3]){
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
			if(strcmp(pararecibir,"3") == 0) printf("Es el turno del hijo 3\n");		
		}
		close(PaH3[0]);
		close(H3aP[1]);
	}
	else printf("nandakorewa\n");
	exit(1);
    return 0;
}