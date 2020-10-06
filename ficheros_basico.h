#include <limits.h>
#include "bloques.h"
#include <time.h>

#define posSB 0 //[initSB] Posición del bloque donde se aloja la estructura del superbloque.
#define tamSB 1 //[initSB] Amplitud de bloques que abarcara el superbloque.
#define INODOSIZE 128 //En alguna parte fue definido esta variable, ¿es posible que sea "tamañao INODO" con 128?
#define minbloquesmkfs 4 //Especifica la cantidad minima de bloques que puede generar.

struct superbloque {
    unsigned int posPrimerBloqueMB; //Posición del primer bloque del mapa de bits en el SF.
    unsigned int posUltimoBloqueMB; //Posición del último bloque del mapa de bits en el SF.
    unsigned int posPrimerBloqueAI; //Posición del primer bloque del array de inodos en el SF.
    unsigned int posUltimoBloqueAI; //Posición del último bloque del array de inodos en el SF.
    unsigned int posPrimerBloqueDatos; //Posición del primer bloque de datos en el SF.
    unsigned int posUltimoBloqueDatos; //Posición del último bloque de datos en el SF.
    unsigned int posInodoRaiz; //Posicion relativa en el AI la ubicación del inodo raíz del SF.
    unsigned int posPrimerInodoLibre; //Posicion relativa en el AI el proximo inodo disponible (vacio).
    unsigned int cantBloquesLibres; //El número de bloques sin ser reservados.
    unsigned int cantInodosLibres; //El número de inodos sin ser reservados.
    unsigned int totBloques; //Cantidad máxima de bloques generados en el SF (bastante importante, es el sizeof del fichero).
    unsigned int totInodos; //Cantidad máxima de inodos generados en el SF.
    char padding[BLOCKSIZE-12*sizeof(unsigned int)]; //Relleno restante para abarcar 1 bloque el superbloque.
};

struct inodo {
    char tipo; //Una letra que define el tipo de inodo { "l: libre", "d: carpeta", "f: fichero" } //ghex char en el offset+0
    unsigned char permisos; //Un valor entero del permiso sobre un monousuario, la suma de { "r: +4", "w: +2", "x: +1" }
    //Por cuestiones internas de alineación de estructuras, si se está utilizando
    //· Un tamaño de palabra de *4 bytes* (microprocesadores de 32 bits):
    //unsigned char reservado_alineacion1 [2];
    //· Y un tamaño de palabra de *8 bytes* (microprocesadores de 64 bits): 
    //unsigned char reservado_alineacion1 [6];
    char reservado_alineacion1[6];
    time_t atime; //Acceso a los datos, fecha/hora.
    time_t mtime; //Modificacion de los datos, fecha/hora.
    time_t ctime; //Creacion de los datos, fecha/hora.
    
    //comprobar el tamaño del tipo time_t para vuestra plataforma/compilador:
    //printf ("sizeof time_t is: %d\n", sizeof(time_t));
    
    unsigned int nlinks; //Cantidad de enlaces que apuntan a este inodo. Si llega a cero, se ha de liberar.
    unsigned int tamEnBytesLog; //Tamaño que abarca los bytes lógicos.
    unsigned int numBloquesOcupados; //Cuantos bloques usa el inodo.
    unsigned int punterosDirectos[12]; //Punteros de bloques de datos. //ghex directo[0] en el offset+44 [45 bytes]
    unsigned int punterosIndirectos[3]; //Punteros sobre otro inodo indexado; simple, doble, triple.
    
    // Utilizar una variable de alineación si es necesario para vuestra plataforma/compilador.
    //CUIDADO: Se debe de restar en el padding en el caso de usar más variables.
    char padding[INODOSIZE-2 * sizeof(unsigned char)-3 * sizeof(time_t)-18 * sizeof(unsigned int)-6 * sizeof(unsigned char)];
    //char padding[INODOSIZE-8*sizeof(unsigned char)-3 * sizeof(time_t)-18 * sizeof(unsigned int)]; //Linea referida en años anteriores.
};

int bmountSAFE(const char *camino); //Delega la función bmount y verifica que la unidad sea válida.
int integridadDisco(unsigned int informarError); //Inspecciona si contiene errores en el superbloque, útil para comprobar el bmount o prevenir errores posteriores.
int tamMB(unsigned int nbloques); //Cálcula la cantidad de bloques que va a ocupar el mapa de bits.
int tamAI(unsigned int ninodos); //Cálcula la cantidad de bloques que va a ocupar el array de inodos.
int initSTRUCT(unsigned int nbloques); //Ejecuta "initSB", "initMB", "initAI", ocupar los bloques de estos 3 y definir el inodo raíz (sólo para mi_mkfs).
int initSB(unsigned int nbloques, unsigned int ninodos); //Formatear el superbloque de la unidad (sólo para mi_mkfs).
int initMB(); //Formatear el mapa de bits (sólo para mi_mkfs).
int initAI(); //Formatear el array de inodos (sólo para mi_mkfs).
int escribir_bit(unsigned int nbloque, unsigned int bit); //Cambia el estado libre/ocupado sobre un bloque de datos (o como punteros).
unsigned char leer_bit(unsigned int nbloque); //Permite conocer si un bloque de datos (o como punteros) esta en uso.
int reservar_bloque(); //Extrae un bloque de datos disponible para guardar datos (o como punteros).
int liberar_bloque(unsigned int nbloque); //Retira un bloque físico de datos y lo vuelve a ponerlo como disponible en el mapa de bits.
int escribir_inodo(unsigned int ninodo, struct inodo inodo); //Actualizar el contenido del inodo (metadatos) y sus punteros.
int leer_inodo(unsigned int ninodo, struct inodo *inodo); //Leer el contenido del inodo (metadatos) y sus punteros.
int reservar_inodo(unsigned char tipo, unsigned char permisos); //Extrae un inodo disponible para ser usado y lo formatea.
int obtener_nRangoBL(struct inodo inodo, unsigned int nblogico, unsigned int *ptr); //Obtiene el bloque lógico a procesar.
int obtener_indice (int nblogico, int nivel_punteros); //Permite conocer la ubicación según el nivel de profundidad.
int traducir_bloque_inodo(unsigned int ninodo,unsigned int nblogico, char reservar); //Recorre a través los punteros lógicos al bloque de datos físicos.
int liberar_bloques_inodoLEGACY(unsigned int ninodo, unsigned int nblogico); //Version original del liberar (la 'no' recursiva).
int liberar_bloques_inodoRECURSIVO(unsigned int ninodo, unsigned int nblogico); //Version recursiva personalizada del liberar.
int liberador_punteros_datosRECURSIVO(unsigned int bloquepuntero, unsigned int bloqueminimo, int nivel_punteros, int nblimite, int *nliberados, int *nprocesados); //Accionado por "liberar_bloques_inodoRECURSIVO", anexa recursivamente los tipos de bloque y los va liberando.
int liberar_bloques_inodo(unsigned int ninodo, unsigned int nblogico); //Procedimiento de liberar, la vía en como se libera depende del ajuste del z_EXTRA "LIBERAR_RECURSIVO".
int liberar_inodo(unsigned int ninodo); //Liberar TODOS los bloques que usan el inodo y lo marca como libre.