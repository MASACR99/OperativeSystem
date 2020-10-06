#include "simulacion.h"

struct REGISTRO registro;
unsigned int NUMPROCESOS = 100;
unsigned int REGMAX = 500000; //limitado en la simulacion
unsigned int NESCRITURAS = 50;
unsigned int tEsperaProceso = 50000;
unsigned int tGenerProc = 200000;
unsigned int acabados = 0;

int main(int argc, char **args){
    char tiempo[BLOCKSIZE];
    char caminodir[BLOCKSIZE];
    char caminoaux[BLOCKSIZE];
    char buffer[BLOCKSIZE];
    __pid_t pid;

    if(argc < 1){
        fprintf(stderr, MSG_ERROR("simulacion.c") "La correcta llamada a simulacion deberia ser: './simulacion <disco>' \n");
        return -1;
    }
    bmountSAFE(args[1]); //Parte desde un estado inicial (unidad ya existente), necesita comprobar que sea estable.
    //crear directorio raiz /simul_aaaammddhhmmss/
    strcpy(caminodir, "/simul_"); //funcion de time.h que permite realizar un string con los datos de tiempo
    strcat(caminodir, current_time());
    strcat(caminodir, "/");
    if(mi_creat(caminodir,'7','2') == -1){
        bumount();
        fprintf(stderr, MSG_ERROR("simulacion.c") "mi_creat ha tenido un error\n");
        return -1;
    }
    signal(SIGCHLD, reaper);
    for(int proceso = 1; proceso <= NUMPROCESOS; proceso++){
        pid = fork();
        if(pid == 0){
            bmount(args[1]); //montar hijo (se omite la verificación porque ya la ha superado)
            //crear directorio del proceso
            strcpy(caminoaux, caminodir);
            strcat(caminoaux, "proceso_");
            pid = getpid();
            sprintf(tiempo, "%ld/", (long) pid);
            strcat(caminoaux, tiempo);
            if(VERIFICACION_DEV) fprintf(stderr, "proceso[%d] pid_%d anexado a '%s'\n",proceso,pid,caminoaux);
            if(mi_creat(caminoaux,'7','2') == -1){
                bumount();
                fprintf(stderr, MSG_ERROR("simulacion.c") "mi_creat ha tenido un error\n");
                return -1;
            }
            //crear fichero prueba.dat
            strcat(caminoaux,"pruebadat"); //BUGFALL: /prueba.dat provoca corrupcion de memoria en buscar_entrada.
            if(mi_creat(caminoaux,'7','1') == -1){
                bumount();
                fprintf(stderr, MSG_ERROR("simulacion.c") "mi_creat ha tenido un error\n");
                return -1;
            }
            srand(time(NULL)+pid);
            for(int i = 0; i < NESCRITURAS; i++){
                registro.fecha = time(NULL);
                registro.pid = pid;
                registro.nEscritura = i + 1;
                registro.nRegistro = rand() % REGMAX;
                memset(buffer,0,sizeof(struct REGISTRO));
                mi_write(caminoaux,&registro,registro.nRegistro * sizeof(struct REGISTRO),sizeof(struct REGISTRO)); //escribir el registro en registro.nRegistro * sizeof(struct registro)
                usleep(tEsperaProceso); //esperar tEsperaProceso milisegundos
            }
            memset(caminoaux, 0, 256);
            bumount();
            exit(0);
        }
        usleep(tGenerProc);
    }
    while (acabados < NUMPROCESOS){
        pause();
    }
    bumount();
    exit(0);
}

void reaper(){
    pid_t ended; //se puede usar para mostrar el proceso terminado
    signal(SIGCHLD,reaper);
    while((ended = waitpid(-1,NULL,WNOHANG))>0){
        acabados++;
    }
}

char* current_time(){
    /*Funcion que devuelve el char que irá seguido de 'simul_' para la carpeta de simulacion*/
    time_t ct;
    char* dest;

    time(&ct);
    dest = malloc(14);

    strftime(dest, 14, "%Y%m%d%H%M%S", localtime(&ct));
    return dest;
}