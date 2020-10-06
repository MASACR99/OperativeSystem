#include "directorios.h"

int main (int argc, char** argv)
{
    const char *nombre_fichero; //Unidad dispositivo de bloques.
    int inodoREFERIDO = -1; //Hay que reconocer el número del inodo, necesito recoger su valor desde el "mi_stat()".
    struct STAT stat; //Requerido por "mi_stat()" en directorios.c

    if(argc < 3) { //Argumentos obligatorios para que funcione a nivel básico.
        fprintf(stderr, MSG_FATAL("mi_stat.c") "Sintaxis: mi_stat <nombre_dispositivo> </ruta>\n");
        fprintf(stderr, "</ruta> siempre ha de empezar por '/' y exista el fichero/directorio (incluyendose los intermedios).\n");
        exit(EXIT_FAILURE);
    }

    nombre_fichero = argv[1]; //Unidad dispositivo de bloques.
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.

    if(mi_stat(argv[2], &stat, &inodoREFERIDO) >= 0 && inodoREFERIDO >= 0) { //Si es aceptador, tengo un inodo.
        struct tm *ts; //Usado para el palo de la fecha.
        char atime[80]; //String para la fecha.
        char mtime[80]; //String para la fecha.
        char ctime[80]; //String para la fecha.
        DEFINIR_SELLO_RWX; //Importar del z_EXTRA la variable array "SELLO_RWX".
        printf(MSG_INFO("Nº de inodo") "%d\n", inodoREFERIDO); //Inodo ordinario.
        if(stat.tipo == 'd') printf(MSG_VAR("tipo") TVAR(COLOR_AZUL, "%c") "\n", stat.tipo); //El tipo de inodo. 'd' directorio/carpeta
        else printf(MSG_VAR("tipo") "%c\n", stat.tipo); //El tipo de inodo. 'l' libre, 'f' fichero
        //Parte de los permisos y color.
        if(stat.permisos >= 4 && stat.permisos <= 7) printf(MSG_VAR("permisos") "'" TVAR(COLOR_VERDE, "%s") "' (%d)\n", SELLO_RWX[(int)stat.permisos], stat.permisos);
        else if(stat.permisos > 0) printf(MSG_VAR("permisos") "'" TVAR(COLOR_AMARILLO, "%s") "' (%d)\n", SELLO_RWX[(int)stat.permisos], stat.permisos);
        else printf(MSG_VAR("permisos") "'" TVAR(COLOR_CHILLON, "%s") "' (%d)\n", SELLO_RWX[0], stat.permisos);
        ts = localtime(&stat.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf(MSG_VAR("atime") "%s\n", atime);
        ts = localtime(&stat.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf(MSG_VAR("mtime") "%s\n", mtime);
        ts = localtime(&stat.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
        printf(MSG_VAR("ctime") "%s\n", ctime);
        printf(MSG_VAR("nlinks") "%d\n", stat.nlinks); //Cantidad de enlaces referidos sobre este inodo (cuantas referencias es anti-imagen hacia este inodo).
        printf(MSG_VAR("tamEnBytesLog") "%d\n", stat.tamEnBytesLog); //Contenido máximo escrito (posible offset entre ellos).
        printf(MSG_VAR("numBloquesOcupados") "%d\n", stat.numBloquesOcupados); //Cantidad de bloques de datos y de punteros esta dando uso.
        printf("\n"); //Linea final.
    }

    bumount(); //Finalizar el proceso.
    return 0;
}