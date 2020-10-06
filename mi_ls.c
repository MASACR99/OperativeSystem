#include "directorios.h"
#include <ctype.h> // tolower() [cambiar a minuscula el caracter "si fuera aplicable"]

int main(int argc, char **args)
{
    const char *nombre_fichero; //Unidad dispositivo de bloques.
    char *buffer; //(BUGFALL) Si hay demasiadas entradas, se podría colapsar. Es el buffer cache que le retorna "mi_dir"
    int nentradas = 0; //Cantidad de entradas retornadas.
    int retornoLIB = -1; //Usado para ver si experimento un error al ejecutar la libreria.
    int magnitudACUMULAR = 72; //Valor base "del peor de los casos". (60 nombre, 1 para el \n, 11 para insertar COLOR). Este determina como de grande será el malloc.
    char quickPrint[40]; //El máximo posible que van a abarcar las cabeceras son "39".
    memset(quickPrint, 0, 40); //Replicar el valor, esto esta para que aparezca el formato dinamico al mostrarlo.

    //Variables de sintaxis para extension (ir agregando a medida que se vaya diseñando).
    unsigned int extensionX = 0; //Ajuste adicional de eXtension. (-x)
    unsigned int extensionI = 0; //Ajuste adicional para mostrar el número del inodo. (-i)


    if(argc < 3) { //Argumentos obligatorios para que funcione a nivel básico.
        fprintf(stderr, MSG_FATAL("mi_ls.c") "Sintaxis: mi_ls <nombre_dispositivo> </ruta> #extension#\n");
        fprintf(stderr, "Extensión [se puede combinar en el mismo '-']: (ausencia) sólo nombres, '-x' modo lista detallada, '-i' mostrar número de inodo.\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 4) { //Tengo un argumento eXtension, determinar que es...
        int longitudEXTENSION = strlen(args[3]); //Contar el número de caracteres del #extension#.
        if(longitudEXTENSION > 0) { //Contiene algo escrito...
            if(args[3][0] == '-') { //Declara el formato extension, pasa a modo lectura de ajustes.
                char temporal; //Cambiar a minuscula
                for(unsigned int i = 1; i < longitudEXTENSION; i++) { //Inspeccionar los caracteres de #extension#
                    temporal = tolower(args[3][i]); //Casting a minuscula.
                    if(temporal == 'x') extensionX = 1; // '-x' habilitar modo lista detallada
                    if(temporal == 'i') extensionI = 1; // '-i' habilitar la visualización de 'ninodo'.
                }
            } else { //Argumento de #extension# mal formulada.
                fprintf(stderr, MSG_ERROR("mi_ls.c") "Error sintactico al procesar la extensión, ha de empezar con el caracter '-' y luego el caracter de ajuste\n");
                fprintf(stderr, "Extensión [se puede combinar en el mismo '-']: (ausencia) sólo nombres, '-x' modo lista detallada, '-i' mostrar número de inodo..\n");
                fprintf(stderr, "EJEMPLO: '" TVAR(COLOR_AMARILLO, "mi_ls disco / -X") "'\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    nombre_fichero = args[1]; //Unidad dispositivo de bloques.
    bmountSAFE(nombre_fichero); //Activar y montar la unidad.

    //(BUGFIX) Prevenir overflow, reservando (en la peor circustancia de 60 en nombre TODOS) por cada una factible. 
    retornoLIB = entradasContenidas((char *)args[2]); //Obtener la cuenta de entradas.
    if(retornoLIB > 0) {
        //Preparar la reserva de espacio del buffer. Ademas de precargar los printfs finales.
        if(extensionI) { //Bytes del número del inodo.
            magnitudACUMULAR += 4;
            strcat(quickPrint,"Inodo\t"); //Formato de cabecera del "inodo"
        }
        if(extensionX) { //Lo que genera el modo lista expandida.
            magnitudACUMULAR += 29;
            strcat(quickPrint,"Permisos\tmTime\t\t\tTamaño\t\t"); //Formato de cabecera del "eXtensión"
        }
        strcat(quickPrint,"Nombre\n"); //Formato de cabecera indispensable.
        buffer = malloc(magnitudACUMULAR*retornoLIB); //Designa "el peor caso posible" de todas las entradas que tendrá que hacer frente.
        memset(buffer, 0, magnitudACUMULAR*retornoLIB); //Limpia el contenido del buffer.

        //Programa mi_ls para "directorios"
        nentradas = mi_dir((char *)args[2], buffer, (unsigned char)extensionX, (unsigned char)extensionI); //Se sabe que tengo entradas.
        printf(TVAR(COLOR_AZUL, "Total entradas: ") TVAR(COLOR_AMARILLO, "%d") "\n", retornoLIB); //Mensaje obligatorio.
        if(nentradas > 0) { //Mostrar este campo si hubiera algo que revelar.
            printf( TVAR(COLOR_CYAN, "%s") ,quickPrint); //Colorear toda la cabecera.
            printf("------------------------------------------------------------------------------------\n");
            printf("%s",buffer);
            printf("\n"); //Me faltaba el salto de linea (margen)
        }
    } else {
        printf(TVAR(COLOR_AZUL, "Total entradas: ") TVAR(COLOR_CHILLON, "%d") "\n", retornoLIB); //Mostrar la totalidad de entradas en color rojo.
        if(WARNINGS_EXE) fprintf(stderr, MSG_AVISO("mi_ls.c") "La entrada del '" TVAR(COLOR_CHILLON, "%s") "' no muestra ninguna entrada, ¿has revisado la ruta y tenga algo?.\n",args[2]); //Mensaje de error si se habilita.
    }
    bumount(); //Finalizar el proceso.
    return 0;
}