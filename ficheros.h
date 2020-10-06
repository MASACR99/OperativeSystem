#include "ficheros_basico.h"

//Posteriormente ya se ubicará en el Makefile, ahora no porque no hay nada.

struct STAT {
    //Esta usa la misma estructura del inodo, con la diferencia que no incorpora "punteros (in)directo(s)".
    char tipo; //Una letra que define el tipo de inodo { "l: libre", "d: carpeta", "f: fichero" } //ghex char en el offset+0
    char permisos; //Un valor entero del permiso sobre un monousuario, la suma de { "r: +4", "w: +2", "x: +1" }
    char reservado_alineacion1[6]; //Arquitectura de la CPU x64. Habria que mirar como combinar la de 32.
    time_t atime; //Acceso a los datos, fecha/hora.
    time_t mtime; //Modificacion de los datos, fecha/hora.
    time_t ctime; //Creacion de los datos, fecha/hora.
    unsigned int nlinks; //Cantidad de enlaces que apuntan a este inodo. Si llega a cero, se ha de liberar.
    unsigned int tamEnBytesLog; //Tamaño que abarca los bytes lógicos.
    unsigned int numBloquesOcupados; //Cuantos bloques usa el inodo.
    unsigned char padding[BLOCKSIZE - (3*sizeof(unsigned int)+3*sizeof(time_t)+2*sizeof(char)+6*sizeof(char))];
};

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes);