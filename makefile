run: jugar
	./jugar

clean:
	rm main.o juego.o archivos.o pipes.o jugar archivos.h.gch juego.h.gch pipes.h.gch
	rm -rf mazo
	rm -rf mano1
	rm -rf mano2
	rm -rf mano3
	rm -rf mano4
	rm -rf pozo

com: main.o juego.o archivos.o pipes.o
	gcc -Wall main.o juego.o pipes.o archivos.o -o jugar

main.o: main.c
	gcc -Wall -c main.c

juego.o: juego.c juego.h
	gcc -Wall -c juego.c juego.h

archivos.o: archivos.c archivos.h
	gcc -Wall -c archivos.c archivos.h

pipes.o: pipes.c pipes.h
	gcc -Wall -c pipes.c pipes.h
