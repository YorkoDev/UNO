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

    int p;

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
    p = creacionprocesos();
    printf("[");
    for(p = 0; p < 4-1; p++) printf("%d,",turnos[p]);
    printf("%d]\n",turnos[p]);
    return 0;
}