#include <stdio.h> //printf(), fprint(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR, off_t
#include <stdlib.h> //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h> //errno
#include <string.h> //strerror()
#include "z_EXTRA.h"

#define BLOCKSIZE 1024 //bytes
#define BLOCKSIZEOF BLOCKSIZE*8-1 //Usado cuando se use el "sizeof" de un bloque. El -1 esta por el fallo de línea, posiblemente el EOF

int bmount(const char *camino); //Abre el fichero creado previamente del bmountMK.
int bmountMK(const char *camino); //Esta funcion debería de ser usada en mi_mkfs. Permite crear el fichero.
int bumount(); //Cierra el fichero abierto (o al menos probar).
int bwrite(unsigned int nbloque, const void *buf); //Escribe en una determinada posicion los datos.
int bread(unsigned int nbloque, void *buf); //Leer en una determinada posicion los datos.
off_t tamUnidad(); //Comprueba el tamaño que tiene el fichero/unidad, es compatible con ficheros muy densos.
void mi_waitSem(); //Evita la interrupción
void mi_signalSem(); //Permite la interrupción