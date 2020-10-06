#include "directorios.h"

int main(int argc, char **argv) {

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    const char *rutaOrigen; //Ruta de origen para clonar los objetos.
    const char *rutaDestino; //Ruta de destino donde se genera la entrada.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.

    if (argc < 4)
    { // Requerir sus 4 argumentos
        fprintf(stderr, MSG_FATAL("mi_cp_f.c") "Sintaxis: mi_cp_f <nombre_dispositivo> </ruta origen> </ruta destino>.\n");
        fprintf(stderr, "</ruta origen> siempre ha de empezar por '/' y exista el fichero (incluyendose los intermedios).\n");
        fprintf(stderr, "</ruta destino> siempre ha de empezar por '/' y exista los directorios intermedios (excepto el ultimo, ha crear).\n");
        fprintf(stderr, "NOTA: Este ejecutable sólo se podrá usar exclusivamente en ficheros con lectura y escritura; para directorios use 'mi_cp'.\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    rutaOrigen = argv[2]; //Ruta fuente...
    rutaDestino = argv[3]; //Donde ubicarlo...

    if ((strcmp(rutaOrigen, "/") == 0) || (strcmp(rutaDestino, "/") == 0)) { //Produce anomalias cuando se involucra el inodo raíz.
        fprintf(stderr, MSG_ERROR("mi_cp_f.c") "Error de paradoja ciclica: No se puede usar el directorio raíz como fuente/destino.\n");
        exit(EXIT_FAILURE);
    }

    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_copy_f(rutaOrigen, rutaDestino); //Mira si tuvo algún error (para no mostrar el mensaje de confirmación).
    if(RETURN_EXE && retornoLIB >= 0) fprintf(stderr, MSG_OK("mi_cp_f.c") "Se ha copiado todo de '" TVAR(COLOR_AMARILLO, "%s") "' a la ruta '" TVAR(COLOR_VERDE, "%s") "'.\n",rutaOrigen,rutaDestino); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.
    return 0;
}