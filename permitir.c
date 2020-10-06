#include "ficheros.h"

const char *nombre_archivo;
int ninodo;
int permisos;

int main (int argc, char **argv){
    if(argc < 3){
        fprintf(stderr, MSG_ERROR("permitir.c") "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return(-1);
    }else{
        nombre_archivo = argv[1];
        ninodo = atoi(argv[2]); //Usar atoi... los valores de String son diferentes a los enteros.
        permisos = *((unsigned char*) argv[3]);//atoi(argv[3]); //Usar atoi... los valores de String son diferentes a los enteros.
        bmount(nombre_archivo);
        mi_chmod_f(ninodo,permisos);
        bumount();
    }
}