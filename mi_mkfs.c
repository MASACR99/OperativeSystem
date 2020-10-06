#include "mi_mkfs.h"

int main(int argc, char **argv){

    unsigned char buffer[BLOCKSIZE]; //El objeto bloque.
    int cantidad_bloques; //Número de bloques a crear.

    if(argc != 3) { //$1-mi_mkfs $2-NOMBRE $3-NUMBLOQUES
        fprintf(stderr, MSG_FATAL("mi_mkfs.c") "Sintaxis: mi_mkfs <nombre_fichero> <capacidad_de_bloques>\n");
        return -1; //Irrumpir.
    }

    cantidad_bloques=atoi(argv[2]); //Reconvierte el String en un valor entero, puede generar error y asignar el valor "-1", es necesario el IF para ponerle freno.
    if(cantidad_bloques < minbloquesmkfs) { //Puede funcionar con un minimo de 4 bloques o superior. Tambien impide que "atoi" experimente error, si hay decimas las ignora.
        fprintf(stderr, MSG_ERROR("mi_mkfs.c") "El parametro <capacidad_de_bloques> necesita un valor entero y como minimo sea de '4' o superior.\n");
        exit(EXIT_FAILURE); //La estructura saldría mal, abortar inmediatamente.
    }
    bmountMK(argv[1]); //La variante MK concede el derecho a crear un fichero nuevo (es lo que se espera).
    memset(buffer, 0 , BLOCKSIZE); //Asigna todo el array al valor 0.
    if(MODO_DEV) printf(MSG_INFO("mi_mkfs.c") "Escritura de bloques %d bytes por lote.\n",BLOCKSIZE); //Opcionalmente, notifica el tamaño del blocksize (en bytes).
    for(int i = 0; i < cantidad_bloques; i++){ //Generar cada bloque, compuesto de 0's.
        bwrite(i, buffer); //A cada bloque... lo escribo en el fichero.
        memset(buffer, 0 , BLOCKSIZE); //Reasigna todo el array al valor 0 (por si acaso).
    }
    initSTRUCT(cantidad_bloques); //Hace el equivalente a "initSB, initMB, initAI, reservar_bloques" (ficheros_basico.c)
    bumount(); //Desmontar la unidad creada.
    if(RETURN_DEV) printf(MSG_OK("mi_mkfs.c") "Unidad de bloques '%s' creada con exito, capacidad de %d.\n",argv[1],cantidad_bloques); //Muestra una confirmación (si esta habilitada...)

    return 0;
}