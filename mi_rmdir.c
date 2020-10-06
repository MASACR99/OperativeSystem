#include "directorios.h"


int main(int argc, char **argv) {

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.

    if (argc < 3)
    { // Requerir sus 3 argumentos
        fprintf(stderr, MSG_FATAL("mi_rmdir.c") "Sintaxis: mi_rmdir <nombre_dispositivo> </ruta>.\n");
        fprintf(stderr, "</ruta> siempre ha de empezar por '/' y exista el directorio (incluyendose los intermedios).\n");
        fprintf(stderr, "NOTA: Este rmdir sólo elimina únicamente directorios vacios.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[2], "/") == 0) {
        fprintf(stderr, MSG_ERROR("mi_rmdir.c") "Error: El directorio raíz no puede ser eliminado.\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_rmdir(argv[2], 0); //Mira si tuvo algún error (para no mostrar el mensaje de confirmación)
    if(RETURN_EXE && retornoLIB >= 0) fprintf(stderr, MSG_OK("mi_rmdir.c") "Se ha suprimido el directorio '" TVAR(COLOR_AMARILLO, "%s") "'.\n",argv[2]); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.
    return 0;
}