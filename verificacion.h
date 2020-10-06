#include "simulacion.h"

#define NUMPROCESOS 100

struct INFORMACION{
    int pid;
    unsigned int nEscrituras; //validadas, por lo general 50, poco probable 49, muy poco 48, 47 mmm NOPE
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};