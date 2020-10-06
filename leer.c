#include "ficheros.h"

//Esta variable se define aquí ya que posteriormente la profesora intentará ver si el programa funciona igual si cambiamos el tamaño del buffer de lectura. 
//ATENTOS: Genera stack smashing (overflow cuando es BUFSIZE < BLOCKSIZE); BUGFALL: Un valor 0 causa errores en bread; y menores de 10 puede incluso durar excesivamente; hasta un máximo de 20 veces inferior es RAZONABLE.
#define BUFSIZE 1500

int main(int argc, char** args)
{
    struct inodo inodo;
    unsigned int offset = 0, totales = 0, lengthResultado = 1024, BUGFALLSIZE = 0; //'BUGFALLSIZE' Aplicar corrector para el stack smashing overflow (< BLOCKSIZE).
    int bytes = 0; //No hace falta explicar que es.
    char buffer[BUFSIZE];
    char fixbuffer[BLOCKSIZE]; //Previene el overflow por un bloque inferior.
    char* resultado = malloc(lengthResultado);

    //Checkeamos sintaxis
    if (argc < 2)
    {
        fprintf(stderr, MSG_ERROR("leer.c") "Sintaxis: leer <dispositivo> <número de inodo>.\n");
        return -1;
    }
    if(BUFSIZE < BLOCKSIZE) { //Problema del stack smashing inminente, hay que cambiar.
        if(BUFSIZE < BLOCKSIZE/20 || BUFSIZE <= 0) { //BUGFIX: La máquina podría tener problemas (con valor 0) o durar excesivamente (1 hasta 10), lo restringe hasta un máximo de 20 que es razonable.
            fprintf(stderr, MSG_ERROR("leer.c") "CONFIGURACION INVALIDA, se requiere un BUFSIZE de '%d' o superior para prevenir colapso.\n",BLOCKSIZE/20);
            return -1;
        }
        BUGFALLSIZE = 1; // Poner en modo TRUE.
        if(WARNINGS_EXE) fprintf(stderr, MSG_AVISO("leer.c") "BUFSIZE '%d' es menor que el de BLOCKSIZE '%d', arreglo overflow sometido.\n",BUFSIZE,BLOCKSIZE); //Pongo un ejemplo así para que no de un warning.
        //Usar como ayuda. //if(BUGFALLSIZE) {} else {}
    }
    //Montamos el dispositivo
    bmount((const char*) args[1]);
    //Limpiamos el buffer
    memset(buffer, 0, BUFSIZE); //Prelimpieza.
    memset(fixbuffer, 0, BLOCKSIZE); //Prelimpieza para la correctora.
    //Hacemos el bucle de lectura mientras vamos imprimiendo el texto que leemos del inodo
    int inodoParam = atoi(args[2]); //El número del inodo.
    //bytes = mi_read_f(*((unsigned int*)args[2]), buffer, offset, BUFSIZE);
    if(BUGFALLSIZE) { //Por si se intenta colapsar (overflow) de la lectura.
        bytes = mi_read_f(inodoParam, fixbuffer, offset, BUFSIZE); //Cuando se somete al arreglo, necesita 1 bloque entero, aún así esta limitado a escribir X bytes.
    } else { //Tradicional.
        bytes = mi_read_f(inodoParam, buffer, offset, BUFSIZE); //Admitido la conversion.
    }

    while (bytes > 0)
    {
        if(AMPLIAR_EXE && totales % 2000000 == 0) fprintf(stderr, "\n" MSG_NOTA("leer.c") "Intervalo procesado %d.\n", totales); //Ir mostrando como evoluciona los pasos intermedios (una buena forma de saber que no se ha calado).
        if(BUGFALLSIZE) memcpy(buffer, fixbuffer, BUFSIZE); //Recoger la PORCIÓN delimitado del mi_read_f.
        //write(1,...) imprime el contenido de buffer por la salida estándar
        write(1, buffer, BUFSIZE);
        memset(buffer,0,BUFSIZE); //Limpiar el buffer que fue usado para que no muestre basura.
        memset(fixbuffer,0,BLOCKSIZE); //Limpiar el buffer de arreglo que fue usado para que no muestre basura.
        offset += BUFSIZE; //Avanzamos de bloque en bloque
        totales += bytes;
        if(BUGFALLSIZE) {
            bytes = mi_read_f(inodoParam, fixbuffer, offset, BUFSIZE); //El mismo arreglo que afuera.
        } else {
            bytes = mi_read_f(inodoParam, buffer, offset, BUFSIZE); //Tradicional.
        }
        if ( bytes < BUFSIZE )  //En el caso de que el último bloque ocupe menos de un BUFSIZE, lo recortaremos
        // Este arreglo es para que haya concordancia entre los bytesLog del fichero y los bytes que ocupe
        // El fichero cuando leamos el inodo y lo redireccionemos a éste. Incluye el arreglo de bloques diminutos.
        {
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
        //printf("offset '%d', totales '%d', bytes '%d', limite '%d'\n",offset,totales,bytes,limite); //!!! Quitar eso tras probar que funciona.
    }
    //Leemos el inodo que nos pasan por parámetro para comprobar que los tamaños coinciden
    leer_inodo(inodoParam, &inodo); //No usar el ATOI si ya la tenias definida, causaba fallos en raras ocasiones.
    /*write(2,...) imprime el contenido del buffer por la salida estándar de errores*/
    memset(buffer,0,BUFSIZE); //Limpia para que no muestre basura.
    memset(resultado,0,lengthResultado); //Limpia para que no muestre basura.
    //sprintf(resultado, "\nBytes leídos: %d. Inodo.tamEnBytesLog: %d.\n", totales, inodo.tamEnBytesLog);
    if(totales == inodo.tamEnBytesLog) sprintf(resultado, "\nBytes leídos: " TVAR(COLOR_VERDE,"%d") ". Inodo.tamEnBytesLog: " TVAR(COLOR_AMARILLO,"%d") ".\n", totales, inodo.tamEnBytesLog); //Comprobación válida.
    else sprintf(resultado, "\nBytes leídos: " TVAR(COLOR_ROJO,"%d") ". Inodo.tamEnBytesLog: " TVAR(COLOR_AMARILLO,"%d") ".\n", totales, inodo.tamEnBytesLog); //El procesamiento tuvo un error.
    write(2, resultado, strlen(resultado));
    //Desmontamos el dispositivo
    bumount();

    return 0;
}