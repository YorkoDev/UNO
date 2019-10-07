#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#include "archivos.h"
#include "juego.h"
#include "pipes.h"
/*
un main de prueba para realizar testear las funciones realizadas para este entregable
*/

int main()
{

	mazo();
	iniciar_partida();
	
	jugando();
	return 0;
}