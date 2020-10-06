#include "directorios.h"

int main(int argc, char **argv) {

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    const char *rutaOrigen; //Ruta de origen para mover y retirar de la entrada del padre.
    const char *rutaDestino; //Ruta de destino donde se genera la entrada.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.

    if (argc < 4)
    { // Requerir sus 4 argumentos
        fprintf(stderr, MSG_FATAL("mi_link.c") "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_fichero_destino>.\n");
        fprintf(stderr, "</ruta_fichero_original> siempre ha de empezar por '/' y exista el fichero (incluyendose los intermedios).\n");
        fprintf(stderr, "</ruta_fichero_destino> siempre ha de empezar por '/' y exista los directorios intermedios (excepto el ultimo, ha crear).\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    rutaOrigen = argv[2]; //Ruta fuente...
    rutaDestino = argv[3]; //Donde ubicarlo...
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_link(rutaOrigen, rutaDestino); //Mira si tuvo algún error (para no mostrar el mensaje de confirmación)
    if(RETURN_EXE && retornoLIB >= 0) fprintf(stderr, MSG_OK("mi_link.c") "Se ha creado '" TVAR(COLOR_AMARILLO, "%s") "' haciendo objetivo a la ruta inodo '" TVAR(COLOR_VERDE, "%s") "'.\n",rutaDestino,rutaOrigen); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.
    return 0;
}