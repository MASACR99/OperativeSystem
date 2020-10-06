#include "mi_mkdir.h"
#include "directorios.h"

int main(int argc, char **argv){

    const char *nombre_fichero; //Unidad dispositivo de bloques.
    int nuevopermiso; //Valor de los permisos a redefinir.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.

    if (argc < 4)
    { // Requerir sus 4 argumentos
        fprintf(stderr, MSG_FATAL("mi_mkdir.c") "Sintaxis: mi_mkdir <nombre_dispositivo> <permisos> </ruta>.\n");
        fprintf(stderr, "<permisos> ha de ser un valor entre [0-7] (incluidos).\n");
        fprintf(stderr, "</ruta> siempre ha de empezar por '/' y exista los directorios intermedios (excepto el ultimo, ha crear).\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    nuevopermiso = *((unsigned char*) argv[2]); //Casting a valor, se espera que salga 48 a 55 (incluidos) para tener la certeza de que esta entre [0-7]
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.
    retornoLIB = mi_creat(argv[3], nuevopermiso, '2'); //(FORMATO DIRECTORIO) Mira si tuvo algún error (para no mostrar el mensaje de confirmación)
    DEFINIR_SELLO_RWX; //Importar del z_EXTRA la variable array "SELLO_RWX".
    if(RETURN_EXE && retornoLIB >= 0) fprintf(stderr, MSG_OK("mi_mkdir.c") "Directorio creado, ruta '" TVAR(COLOR_VERDE, "%s") "' con el permiso inicial '" TVAR(COLOR_AMARILLO, "%s") "'.\n",argv[3],SELLO_RWX[nuevopermiso-48]); //Mensaje de confirmacion si se quiere.
    bumount(); //Finalizar el proceso.

    return 0;
}