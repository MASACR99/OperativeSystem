#include "ficheros.h"

const char *nombre_archivo;
struct inodo inodo;
//struct superbloque sb; //No veo ningún uso del superbloque.
int ninodo;
int nbytes;

int main (int argc, char **argv){
    
    //Para mostrar las fechas debidamente.
    struct tm *ts; //Usado para el palo de la fecha.
    char atime[80]; //String para la fecha.
    char mtime[80]; //String para la fecha.
    char ctime[80]; //String para la fecha.

    if(argc < 3){               //Comprobamos la cantidad de parámetros
        fprintf(stderr, MSG_ERROR("truncar.c") "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return(-1);
    }else{
        nombre_archivo = argv[1];   //Guardamos parámetros de entrada
        ninodo = atoi(argv[2]); //Usar atoi... los valores de String son diferentes a los enteros.
        nbytes = atoi(argv[3]); //Usar atoi... los valores de String son diferentes a los enteros.
        //if(argv[3]==0 || argv[3] > 1){  //Comprobamos nbytes correcto
        if(nbytes >= 0){  //Comprobamos nbytes correcto
            bmount(nombre_archivo);
            //bread(0,&sb); //Errr, ¿hace algo?
            leer_inodo(ninodo, &inodo);
            int excesivoEOF=inodo.tamEnBytesLog-nbytes; //Hay un bug que el propio IF no puede regular bien; Elliot.
            int numRetirados = -1; //mi_truncar_f ya se encargá de cambiar este valor si quita o no bloques sin ser causa de EOF.
            if(excesivoEOF >= 0) { //Comprobamos que no sobrepasemos EOF.
                //Puede truncar los valores, ejecutar y revelar los bloques que fueron retirados.
                numRetirados = mi_truncar_f(ninodo,nbytes);
                printf(MSG_NOTA("truncar.c")"Total de bloques liberados: " TVAR(COLOR_AMARILLO,"%d") "\n\n", numRetirados); //Muestra los bloques que fueron liberados
            } else {
                //En caso contrario muestra el error en la cabecera.
                fprintf(stderr, MSG_ERROR("truncar.c") "No se puede truncar más allá del EOF\n\n");
            }
            //Indistintamente si tuvo error o no, ha de revelarse los datos del inodo.
            leer_inodo(ninodo, &inodo); //Hay que volver a leer el inodo porque ha cambiado sus valores.
            printf(MSG_AVISO("DATOS INODO %d") ">>\n", ninodo);
            printf(MSG_VAR("tipo") "%c\n", inodo.tipo);
            printf(MSG_VAR("permisos") "%d\n", inodo.permisos);
            ts = localtime(&inodo.atime);
            strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("atime") "%s\n", atime); //Fecha de acceso.
            ts = localtime(&inodo.mtime);
            strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("mtime") "%s\n", mtime); //Fecha de modificación.
            ts = localtime(&inodo.ctime);
            strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("ctime") "%s\n", ctime); //Fecha de creación.
            printf(MSG_VAR("nlinks") "%d\n", inodo.nlinks);
            printf(MSG_VAR("tamEnBytesLog") "%d\n", inodo.tamEnBytesLog);
            printf(MSG_VAR("numBloquesOcupados") "%d\n", inodo.numBloquesOcupados);
            bumount(); //Una vez hecho, cerramos, ya no tiene más uso.

            if(numRetirados >= 0) { //Si ha podido liberar o no hace cambios, finaliza bien.
                return 0;
            } else { //Este tuvo el error del EOF, finaliza con error.
                return -1;
            }
        }else{
            fprintf(stderr, MSG_ERROR("truncar.c") "Nbytes incorrecto\n");
            return(-1);
        }
    }
}