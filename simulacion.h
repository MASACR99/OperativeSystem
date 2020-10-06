#include <sys/wait.h>
#include <signal.h>
#include "directorios.h"

struct REGISTRO{
    time_t fecha; //fecha de escritura en formato epoch
    pid_t pid; //PID del proceso que lo ha creado
    int nEscritura; //Numero de escritura entre 1 y 50
    int nRegistro; //Número del registo dentro del fichero (0 a REGMAX-1)
};

void reaper();
char* current_time();