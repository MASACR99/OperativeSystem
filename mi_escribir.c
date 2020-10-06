#include "directorios.h"

int main(int argc, char **args)
{
    char *camino, *fichero, *buffer;
    unsigned int length = 0, offset = 0; //Ponlos agrupados los del mismo tipo (optimización).
    int ret = 0;

    if(argc < 4) { //Argumentos obligatorios para que funcione a nivel básico.
        fprintf(stderr, MSG_FATAL("mi_escribir.c") "Sintaxis: mi_escribir <nombre_dispositivo> </ruta> <texto> #offsets#\n");
        fprintf(stderr, "</ruta> siempre ha de empezar por '/' y exista los directorios intermedios; no puede tener '/' al final (sólo ficheros).\n");
        fprintf(stderr, "Offsets (opcional): Indicar en bytes el offset inicial del fichero desde donde se va a escribir. (ausencia si se omite) 0.\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 5) { //Supuestamente tengo offsets para considerarlo.
        offset=atoi(args[4]); //Reconvierte el String en un valor entero. Sólo recopilará números hasta que termine o haya un caracter en medio.
    } else { //Argumento offsets omitido, hay que predefinir la variable.
        offset = 0; //Distancia inicial predeterminada.
    }

    camino = (char *)args[2];
    fichero = (char *)args[1];
    buffer = (char *)args[3];
    length = (unsigned int) strlen(buffer);

    bmountSAFE(fichero);

    ret = mi_write(camino, buffer, offset, length);
    if(ret > 0) { //Me han escrito bytes en la ruta deseada. He de mostrar el mensaje de cuantos se ha escrito (me lo exigen)
        if(length == ret) printf(TVAR(COLOR_CYAN,"Tamaño en bytes del texto introducido:") " " TVAR(COLOR_VERDE,"%d") ".\t" TVAR(COLOR_CYAN,"Bytes escritos:") " " TVAR(COLOR_VERDE,"%d") "  (+" TVAR(COLOR_AMARILLO,"%d") ")\n", length, ret, offset);
        else printf(TVAR(COLOR_CYAN,"Tamaño en bytes del texto introducido:") " " TVAR(COLOR_AMARILLO,"%d") ".\t" TVAR(COLOR_CYAN,"Bytes escritos:") " " TVAR(COLOR_ROJO,"%d") "\n", length, ret);
    }
    bumount(); //Finalizar el proceso. (no te olvides de cerrarlo)
    return 0;
}