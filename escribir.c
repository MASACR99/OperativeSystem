#include "ficheros.h"

int main(int argc, char **args)
{
    unsigned int offsets[5] = {0, 5120, 256000, 30720000, 71680000};
    unsigned int ninodo = 0;
    struct STAT pstat;

    //Para mostrar las fechas debidamente.
    struct tm *ts; //Usado para el palo de la fecha.
    char atime[80]; //String para la fecha.
    char mtime[80]; //String para la fecha.
    char ctime[80]; //String para la fecha.

    if (argc < 3)
    {
        //Número de argumentos no válido
        fprintf(stderr, MSG_ERROR("escribir.c") "Sintaxis: escribir <dispositivo> <texto> <diferentes inodos>.\n");
        fprintf(stderr, "Offsets: 0, 5120, 256000, 30720000, 71680000\n");
        fprintf(stderr, "Si diferentes_inodos=0 se reserva un sólo inodo para todos los offsets.\n");
        fprintf(stderr, "En caso contrario del diferentes_inodos (!=0), se reserva un inodo por cada offset.\n");
        return -1;
    }

    
    int diferentesInodos = atoi(args[3]); //Determina si es ==0 o !=0.
    //int anchuraSizeof = sizeof(args[2]); //Causaba varios warnings.
    int anchuraSizeof = strlen(args[2]); //Longitud del texto introducido.

    //Hay que comprobar si hay algun texto a insertar para evitar el reservar inodo.
    if(anchuraSizeof > 0) {
        printf(TVAR(COLOR_CYAN,"Tamaño en bytes del texto introducido:") " " TVAR(COLOR_VERDE,"%d") "\n",anchuraSizeof);
    } else {
        printf(TVAR(COLOR_AMARILLO,"Tamaño en bytes del texto introducido:") " " TVAR(COLOR_ROJO,"%d") " (¿...?)\n",anchuraSizeof);
        fprintf(stderr, MSG_FATAL("escribir.c") "El argumento <texto> no puede estar vacio.\n");
        return -1;
    }

    //printf(COLOR_VERDE "Tamaño en bytes del texto introducido: %d.\n" COLOR_NEGRO, anchuraSizeof);
    /*for (unsigned int i = 0; i < 5; i++) {
        if(diferentesInodos) {
            mi_write_f(ninodo = reservar_inodo('f', 7), args[2], offsets[i], anchuraSizeof);
            mi_stat_f(ninodo, &pstat);
        } else {
            //
        }
    }*/


    bmount(args[1]); //Montamos el dispositivo
    if (diferentesInodos)     //Diferentes inodos = 1
    {
        for (unsigned int i = 0; i < 5; i++)
        {
            mi_write_f(ninodo = reservar_inodo('f', 6), args[2], offsets[i], anchuraSizeof); //Cuando mi_write_f se acciona y descubre el texto vacio ya aborta, pero no sabe nada acerca del inodo (realidad, fue reservado).
            mi_stat_f(ninodo, &pstat);
            //Mostrar los valores de cada inodo (por medio del STAT)
            printf(MSG_AVISO("DATOS INODO %d") ">>\n", ninodo);
            printf(MSG_VAR("tipo") "%c\n", pstat.tipo);
            printf(MSG_VAR("permisos") "%d\n", pstat.permisos);
            ts = localtime(&pstat.atime);
            strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("atime") "%s\n", atime); //Fecha de acceso.
            ts = localtime(&pstat.mtime);
            strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("mtime") "%s\n", mtime); //Fecha de modificación.
            ts = localtime(&pstat.ctime);
            strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("ctime") "%s\n", ctime); //Fecha de creación.
            printf(MSG_VAR("nlinks") "%d\n", pstat.nlinks);
            printf(MSG_VAR("tamEnBytesLog") "%d\n", pstat.tamEnBytesLog);
            printf(MSG_VAR("numBloquesOcupados") "%d\n", pstat.numBloquesOcupados);
            printf("\n"); //Espaciado
            //printf(MSG_VAR("escribir.c") "Metainfo del inodo escrito: inodo.tamEnBytesLog: %d. inodo.numBloquesOcupados: %d.\n", pstat.tamEnBytesLog, pstat.numBloquesOcupados); //Era un printf, no el fprintf (vaya warning pfff).
            //printf(MSG_VAR("escribir.c") "Metainfo del inodo escrito: inodo.tamEnBytesLog: " TVAR(COLOR_AMARILLO,"%d") ". inodo.numBloquesOcupados: " TVAR(COLOR_CYAN,"%d") ".\n", pstat.tamEnBytesLog, pstat.numBloquesOcupados);
        }
    }
    else //Diferentes inodos = 0
    {
        ninodo = reservar_inodo('f', 6);
        for ( unsigned int i = 0; i < 5; i++)
        {
            mi_write_f(ninodo, args[2], offsets[i], anchuraSizeof); //Cuando mi_write_f se acciona y descubre el texto vacio ya aborta, pero no sabe nada acerca del inodo (realidad, fue reservado).
            mi_stat_f(ninodo, &pstat);
            //Mostrar los valores del propio inodo (por medio del STAT)
            printf(MSG_AVISO("DATOS INODO %d") ">>\n", ninodo);
            printf(MSG_VAR("tipo") "%c\n", pstat.tipo);
            printf(MSG_VAR("permisos") "%d\n", pstat.permisos);
            ts = localtime(&pstat.atime);
            strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("atime") "%s\n", atime); //Fecha de acceso.
            ts = localtime(&pstat.mtime);
            strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("mtime") "%s\n", mtime); //Fecha de modificación.
            ts = localtime(&pstat.ctime);
            strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("ctime") "%s\n", ctime); //Fecha de creación.
            printf(MSG_VAR("nlinks") "%d\n", pstat.nlinks);
            printf(MSG_VAR("tamEnBytesLog") "%d\n", pstat.tamEnBytesLog);
            printf(MSG_VAR("numBloquesOcupados") "%d\n", pstat.numBloquesOcupados);
            printf("\n"); //Espaciado
        }
        //printf(MSG_VAR("escribir.c") "Metainfo del inodo escrito: inodo.tamEnBytesLog: %d. inodo.numBloquesOcupados: %d.\n", pstat.tamEnBytesLog, pstat.numBloquesOcupados); //Era un printf, no el fprintf (vaya warning pfff).
        //printf(MSG_VAR("escribir.c") "Metainfo del inodo escrito: inodo.tamEnBytesLog: " TVAR(COLOR_AMARILLO,"%d") ". inodo.numBloquesOcupados: " TVAR(COLOR_CYAN,"%d") ".\n", pstat.tamEnBytesLog, pstat.numBloquesOcupados);
    }
    bumount();      //Desmontamos el dispositivo
    return 0;
}