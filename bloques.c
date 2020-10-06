#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0; //Descriptor del fichero (fcntl.h)
#ifdef ENTREGA_3
static sem_t *mutex; //Usado para los semaforos de la entrega 3
static unsigned int inside_sc = 0; //Evitar repetición de wait, semaforos de la entrega 3
#endif

//Escribir en el bloque especificado los datos absolutos del bloque.
int bwrite(unsigned int nbloque, const void *buf){
    //---------------------------
    unsigned int writtenBytes = 0;
    //---------------------------

    if(lseek(descriptor, nbloque*BLOCKSIZE, 0)==-1){ //Nos posicionamos, revisamos si hay error.
        fprintf(stderr, MSG_ERROR("bloques.c/bwrite") "Error del lseek, revisar el argumento (¿existe el bloque '%d'?)\n",nbloque);
        return -1;
    }
    if((writtenBytes = write(descriptor, buf, BLOCKSIZE))==-1){ //Luego escribimos los datos, revisando si surge un error.
        fprintf(stderr, MSG_ERROR("bloques.c/bwrite") "Escritura fallida en el bloque '%d'\n",nbloque);
        return -1;
    }
    if(BLOCK_DEV) fprintf(stderr, MSG_NOTA("bloques.c/bwrite") "Bloque escrito: %d [+%d]\n", nbloque, nbloque*BLOCKSIZE);
    return writtenBytes; //Retorna la cantidad de bytes escritos.

}

//Leer el bloque especificado todos los datos contenidos del bloque.
int bread(unsigned int nbloque, void *buf){
    //---------------------------
    unsigned int readBytes = 0;
    //---------------------------

    if(lseek(descriptor, nbloque*BLOCKSIZE, 0)==-1){ //Nos posicionamos, revisamos si hay error.
        fprintf(stderr, MSG_ERROR("bloques.c/bwrite") "Error del lseek, revisar el argumento (¿existe el bloque '%d'?)\n",nbloque);
        return -1;
    }
    if((readBytes = read(descriptor, buf, BLOCKSIZE))==-1){ //Luego hay que leer los datos, revisando si surge un error.
        fprintf(stderr, MSG_ERROR("bloques.c/bread") "Lectura fallida en el bloque '%d'\n",nbloque);
        return -1;
    }
    if(BLOCK_DEV) fprintf(stderr, MSG_NOTA("bloques.c/bread") "Bloque leido: %d [+%d]\n", nbloque, nbloque*BLOCKSIZE);
    return readBytes; //Retorna la cantidad de bytes leidos.
}

//Abre el fichero creado previamente, es necesario que antes de hacerlo haya pasado por el bmountMK().
int bmount(const char *camino){
    #ifdef ENTREGA_3
    //if(fcntl(descriptor, F_GETFD) >= 0) { //IF alternativo -- Trata de ver el flag del descriptor, si hay error es porque aún no fue abierto. Inmune a la interrupción de señales.
    if(descriptor > 0) {
        close(descriptor);
    }
    if(!mutex) {
        mutex = initSem();
        if(mutex == SEM_FAILED) return -1;
    }
    #endif
    descriptor = open(camino, O_RDWR); //Intenta abrir un fichero PREVIAMENTE EXISTENTE.
    if(descriptor == -1){ //Tuvo un error al abrir, ¿has revisado si existe el fichero?
        fprintf(stderr, MSG_FATAL("bloques.c/bmount") "Apertura exclusiva en '%s', FALLIDA: '%s'\n", camino, strerror(errno));
        exit(EXIT_FAILURE); //Irrumpir la ejecución en su totalidad puesto que las otras instrucciones fallarian.
        //return -1; //El exit irrumpe todo el proceso.
    }
    if(RETURN_DEV) fprintf(stderr, MSG_OK("bloques.c/bmount") "Apertura exclusiva en '%s', procesado [%d].\n", camino, descriptor);
    return descriptor; //El fichero esta abierto y disponible.
}

//Abre el fichero o genera uno de nuevo, en un principio SÓLO "mi_mkfs" puede ejecutarlo y darle la estructura.
int bmountMK(const char *camino){
    #ifdef ENTREGA_3
    //if(fcntl(descriptor, F_GETFD) >= 0) { //IF alternativo -- Trata de ver el flag del descriptor, si hay error es porque aún no fue abierto. Inmune a la interrupción de señales.
    if(descriptor > 0) {
        close(descriptor);
    }
    if(!mutex) {
        mutex = initSem();
        if(mutex == SEM_FAILED) return -1;
    }
    #endif
    umask(000); //Corrige los permisos predeterminados al crear el fichero.
    descriptor = open(camino, O_RDWR|O_CREAT, 0666); //Intenta abrir un fichero, incluyendo la capacidad de agregar el fichero con los permisos "rw-rw-rw-".
    if(descriptor == -1){ //Tuvo un error al abrir/crear, ¿se puede escribir en la carpeta indicada?
        fprintf(stderr, MSG_FATAL("bloques.c/bmountMK") "Apertura de creacion en '%s', FALLIDA: '%s'\n", camino, strerror(errno));
        exit(EXIT_FAILURE); //Irrumpir la ejecución en su totalidad puesto que las otras instrucciones fallarian.
    }
    if(RETURN_DEV) fprintf(stderr, MSG_OK("bloques.c/bmountMK") "Apertura de creacion en '%s', procesado [%d].\n", camino, descriptor);
    return descriptor; //El fichero esta abierto y disponible.
}

//Cierra el fichero abierto previamente.
int bumount(){
    if(fcntl(descriptor, F_GETFD) < 0) { //Comprueba si tiene bandera (flag) el descriptor, si hay error es porque ya estaba cerrado. Es inmune a la interrupción de señales/semaforos.
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("bloques.c/bumount") "Cierre de la unidad de bloques OMITIDA, ya estaba cerrado.\n");
        return -1;
    }
    if (close(descriptor) == -1){ //Cierra el fichero abierto. El posible error del descriptor ya lo regula el if previo.
        fprintf(stderr, MSG_FATAL("bloques.c/bumount") "Cierre de la unidad de bloques, FALLIDA: '%s'\n", strerror(errno));
        return -1;
    }
    #ifdef ENTREGA_3
    deleteSem();
    #endif
    if(RETURN_DEV) fprintf(stderr, MSG_OK("bloques.c/bumount") "Cierre de la unidad de bloques, procesado.\n");
    return 0;
}

//Obtiene el tamaño total que abarca el fichero descriptor.
off_t tamUnidad() {
    if(fcntl(descriptor, F_GETFD) >= 0) { //Comprueba si tiene bandera (flag) el descriptor; si no genere error es porque esta abierto y es accesible.
        off_t size; //Permite reconocer una unidad incluso si su tamaño es enorme. Tambien acepta valor negativo.
        size = lseek(descriptor, 0, SEEK_END); //Posicionar en la última posición del fichero para obtener su offset desde el inicio (tamaño total).
        if(BLOCK_DEV) fprintf(stderr, MSG_VAR("bloques.c/tamUnidad") "El tamaño del fichero abarca '%ld' bytes.\n",size);
        return size;
    }
    return -1; //Si esta cerrado o era innacesible, retornar error.
}

#ifdef ENTREGA_3
void mi_waitSem(){
    if(!inside_sc){
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem(){
    inside_sc--;
    if(!inside_sc){
        signalSem(mutex);
    }
}
#endif