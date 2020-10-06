#include "directorios.h"

//Esta variable se define aquí ya que posteriormente la profesora intentará ver si el programa funciona igual si cambiamos el tamaño del buffer de lectura. 
//ATENTOS: Genera stack smashing (overflow cuando es BUFSIZE < BLOCKSIZE); BUGFALL: Un valor 0 causa errores en bread; y menores de 10 puede incluso durar excesivamente; hasta un máximo de 20 veces inferior es RAZONABLE.
//Si BLOCKSIZE vale 1024, el valor minimo es 51 (incluido) para que pueda funcionar.
#define BUFSIZE BLOCKSIZE
//#define BUFSIZE 1500

int main(int argc, char **args)
{
    unsigned int offset = 0, totales = 0, lengthResultado = 1024, BUGFALLSIZE = 0; //'BUGFALLSIZE' Aplicar corrector para el stack smashing overflow (< BLOCKSIZE) [por defecto '0'].
    int bytes = 0; //No hace falta explicar que es.

    char *camino, *fichero;
    char buffer[BUFSIZE]; //Buffer de lectura con el tamaño personalizado (sólo funciona si es IGUAL O SUPERIOR al blocksize)
    char fixbuffer[BLOCKSIZE]; //Previene el overflow por un bloque inferior.
    char* resultado = malloc(lengthResultado);

    if(argc < 3) { //Argumentos obligatorios para que funcione a nivel básico.
        fprintf(stderr, MSG_FATAL("mi_cat.c") "Sintaxis: mi_cat <nombre_dispositivo> </ruta>\n");
        fprintf(stderr, "</ruta> siempre ha de empezar por '/' y exista los directorios intermedios; no puede tener '/' al final (sólo ficheros).\n");
        exit(EXIT_FAILURE);
    }
    if(BUFSIZE < BLOCKSIZE) { //Problema del stack smashing inminente, hay que cambiar.
        if(BUFSIZE < BLOCKSIZE/20 || BUFSIZE <= 0) { //BUGFIX: La máquina podría tener problemas (con valor 0) o durar excesivamente (1 hasta 10), lo restringe hasta un máximo de 20 que es razonable.
            fprintf(stderr, MSG_ERROR("mi_cat.c") "CONFIGURACION INVALIDA: se requiere un BUFSIZE de '%d' o superior para prevenir colapso.\n",BLOCKSIZE/20);
            return -1;
        }
        BUGFALLSIZE = 1; // Poner en modo TRUE.
        if(WARNINGS_EXE) fprintf(stderr, MSG_AVISO("mi_cat.c") "BUFSIZE '%d' es menor que el de BLOCKSIZE '%d', arreglo overflow sometido.\n",BUFSIZE,BLOCKSIZE); //Pongo un ejemplo así para que no de un warning.
    }

    camino = (char *)args[2];
    fichero = (char *)args[1];
    //Limpiamos el buffer
    memset(buffer, 0, BUFSIZE); //Prelimpieza.
    memset(fixbuffer, 0, BLOCKSIZE); //Prelimpieza para la correctora.


    bmountSAFE(fichero);

    if(BUGFALLSIZE) { //Por si se intenta colapsar (overflow) de la lectura.
        bytes = mi_read(camino, fixbuffer, offset, BUFSIZE); //Cuando se somete al arreglo, necesita 1 bloque entero, aún así esta limitado a escribir X bytes.
    } else { //Tradicional.
        bytes = mi_read(camino, buffer, offset, BUFSIZE); //Tradicional y compatible (1 bloque o más).
    }

    while (bytes > 0) {
        if(BUGFALLSIZE) memcpy(buffer, fixbuffer, BUFSIZE); //Recoger la PORCIÓN delimitado del mi_read.
        write(1, buffer, BUFSIZE);
        memset(buffer,0,BUFSIZE); //Limpiar el buffer que fue usado para que no muestre basura.
        memset(fixbuffer,0,BLOCKSIZE); //Limpiar el buffer de arreglo que fue usado para que no muestre basura.
        offset += BUFSIZE; //Avanzamos de bloque en bloque
        totales += bytes;
        if(BUGFALLSIZE) {
            bytes = mi_read(camino, fixbuffer, offset, BUFSIZE); //El mismo arreglo que afuera.
        } else {
            bytes = mi_read(camino, buffer, offset, BUFSIZE); //Tradicional.
        }
        if ( bytes < BUFSIZE ) { //En el caso de que el último bloque ocupe menos de un BUFSIZE, lo recortaremos (es el último bloque a leer).
            char auxBuf[bytes];
            if(BUGFALLSIZE) {
                memcpy(auxBuf, fixbuffer, bytes); //Del arreglo sometido.
            } else {
                memcpy(auxBuf, buffer, bytes); //Tradicional.
            }
            write(1, auxBuf, bytes);
            totales += bytes; //Previsible, faltaba agregar la cuenta tras salir del bucle.
            bytes = -1; //Forzamos la salida del bucle
        }
    }

    memset(buffer,0,BUFSIZE); //Limpia para que no muestre basura.
    memset(resultado,0,lengthResultado); //Limpia para que no muestre basura.

    sprintf(resultado, "\n\nTotal de bytes leídos: " TVAR(COLOR_VERDE,"%d") ".\n", totales); //Debido a que no tengo acceso al inodo, no se puede comprobar si los tamaños coinciden (a diferencia de leer.c)
    write(2, resultado, strlen(resultado));

    bumount();

    return 0;
}
