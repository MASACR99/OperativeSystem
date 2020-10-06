#include "directorios.h"


int main(int argc, char **argv) {

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    const char *ruta; //La ruta definida.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.
    char modoSuprimir = 0; //Determina si procede con el modo recursivo sobre directorios no vacios.

    if (argc < 3)
    { // Requerir sus 3 argumentos
        fprintf(stderr, MSG_FATAL("mi_rm.c") "Sintaxis: mi_rm <disco> </ruta> #-r#.\n");
        fprintf(stderr, "-r (opcional): Si se indica este argumento, mi_rm podrá eliminar la carpeta con todo su contenido.\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    ruta = argv[2]; //Ruta indicada

    if (strcmp(ruta, "/") == 0) {
        fprintf(stderr, MSG_ERROR("mi_rm.c") "Error: El directorio raíz no puede ser eliminado.\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 4) {
        char *recursivo = (char *)argv[3];
        if (strcmp(recursivo, "-r") != 0) {
            fprintf(stderr, MSG_ERROR("mi_rm.c") "El parámetro opcional para suprimir recursivamente es por medio '-r'.\n");
            exit(EXIT_FAILURE);
        } else modoSuprimir = 1;
    }

    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_unlink(ruta, modoSuprimir); //Mira si tuvo algún error (para no mostrar el mensaje de confirmación), variante con recursividad.
    if(RETURN_EXE && retornoLIB >= 0 && modoSuprimir == 0) fprintf(stderr, MSG_OK("mi_rm.c") "Se ha suprimido la ruta '" TVAR(COLOR_AMARILLO, "%s") "'.\n",ruta); //Mensaje de confirmacion si se quiere.
    else if(RETURN_EXE && retornoLIB >= 0 && modoSuprimir != 0) fprintf(stderr, MSG_OK("mi_rm.c") "Se ha suprimido la ruta '" TVAR(COLOR_AMARILLO, "%s#*") "' y todos sus elementos contenidos en el si hubiera.\n",ruta); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.
    return 0;
}