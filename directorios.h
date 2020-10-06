#include "ficheros.h"

struct entrada
{
    char nombre[60];
    unsigned int ninodo;
};

struct UltimaEntrada{
    char camino[512];
    int p_inodo;
};

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo); //Recorta el string por trozos usando como delimitador "/".
int buscar_entrada(const char *camino_parcial,unsigned int *p_inodo_dir, unsigned int *p_inodo,unsigned int *p_entrada, char reservar, unsigned char permisos); //Función base, usar el "commonAPI_pinodo" para asegurarse de no incluir errores comunes.
int commonAPI_pinodo(const char *camino, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos, char tipoinodo); //Revisa los parametros "camino" y "permisos" sean validos, luego se procesa el "buscar_entrada".
int entradasContenidas(const char *camino); //Permite reconocer la cantidad de entradas contenidas.
char* padrePINODOdirAPI(const char *camino); //Forza a formato de camino a fichero para que siempre "p_inodo_dir" sea el padre y la entrada (incluso directorio) este siempre en "p_inodo". Es recomendable guardarlo como una variable String.
int entradasAPI(const char *camino, struct entrada elemento, int operacion, unsigned int ajuste, int celda); //(BAJO CONSTRUCCION) Permite editar la entrada de visualización de inodos o borrarlo.
int is_null_entry(struct entrada entry); //Funcion auxiliar para "mi_dir".
int is_in_cache(const char* camino, unsigned char rnw, unsigned int* point); //Función auxiliar para mi_read y mi_write
int mi_creat(const char *camino, unsigned char permisos, char tipoCrear); //Crear un fichero/directorio en la ruta contenida.
int mi_dir(const char* camino, char* buffer, unsigned char extended, unsigned char plusinodo); //Usado para "mi_ls", guarda en el buffer las entradas encontradas de un directorio.
int mi_stat(const char *camino, struct STAT *p_stat, int *ninodo); //Carga de metadatos del inodo. Puede transferir el número de inodo para futuras referencias.
int mi_chmod(const char* camino, unsigned char permisos); //Cambia los permisos a un fichero/directorio EXISTENTE.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes); //Lee el contenido de un inodo fichero.
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes); //Escribe el contenido a un inodo fichero.
int mi_link(const char* camino1, const char* camino2);  //Enlaza dos ficheros
int mi_move(const char* origen, const char* destino);  //Cambia la ubicación de un inodo a otra.
int mi_rmdir(const char *camino, unsigned char recursivo);  //Elimina directorios.
int mi_unlink_f(const char *camino, unsigned char suprimible);  //Desenlaza dos ficheros
int mi_unlink(const char* camino, unsigned char type);  //Funcion troncal para suprimir y desenlazar ficheros e includo directorios enteros.
int mi_copy_f(const char* origen, const char* destino);  //Copia un fichero de origen a otra ubicación.
int mi_copy(const char* origen, const char* destino);  //Copia el contenido de origen en destino