#include "directorios.h"


int main(int argc, char **argv) {

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    const char *rutaOrigen; //Ruta de origen para mover y retirar de la entrada del padre.
    char modoSuprimir = 0; //Usado para determinar si el inodo necesita que tenga 2 o más enlaces, o pueda eliminarlo si tiene 1 enlace.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.

    if (argc < 3)
    { // Requerir sus 3 argumentos
        fprintf(stderr, MSG_FATAL("mi_unlink.c") "Sintaxis: mi_unlink <nombre_dispositivo> </ruta_fichero_original> #-f#\n");
        fprintf(stderr, "</ruta_fichero_original> siempre ha de empezar por '/' y exista el fichero (incluyendose los intermedios).\n");
        fprintf(stderr, "-f (opcional): Si se indica este argumento, mi_unlink podrá eliminar el fichero aún si tiene 1 nlink.\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 4) { //Revisa el argumento opcional "forzar eliminación"
        char *recursivo = (char *)argv[3];
        if (strcmp(recursivo, "-f") != 0) {
            fprintf(stderr, MSG_ERROR("mi_unlink.c") "El parámetro opcional para suprimir ficheros de único enlace es por medio '-f'.\n");
            exit(EXIT_FAILURE);
        } else {
            modoSuprimir = 1;
        }
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    rutaOrigen = argv[2]; //Ruta del fichero...
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_unlink_f(rutaOrigen, modoSuprimir); //Mira si tuvo algún error (para no mostrar el mensaje de confirmación), variante con recursividad.
    if(RETURN_EXE && retornoLIB >= 0) fprintf(stderr, MSG_OK("mi_unlink.c") "Se ha retirado el enlace '" TVAR(COLOR_AMARILLO, "%s") "'.\n",rutaOrigen); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.
    return 0;
}