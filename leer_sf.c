#include "leer_sf.h"
#include "directorios.h"

struct superbloque sb;

int main(int argc, char **argv) {
    int valorFiltro = -1; //El -1 es para suponer en mostrar exclusivamente el superbloque.
    if(argc < 2) {
        fprintf(stderr, MSG_FATAL("leer_sf.c") "Sintaxis: leer_sf <nombre_fichero> #grado_muestreo#\n");
        fprintf(stderr, "El grado de muestreo: (ausencia) sólo superbloque, '0' TODOS los campos, '1' sólo mapa de bits, '2' sólo array de inodos, '3' sólo el sizeOf estructural, '4' sólo los inodos ocupados.\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 3) { //Tengo un segundo argumento, el grado de muestreo.
        valorFiltro=atoi(argv[2]);
        if(valorFiltro < 0 || valorFiltro > 4) {
            fprintf(stderr, MSG_AVISO("leer_sf.c") "El grado de muestreo puede ser: (ausencia) sólo superbloque, '0' TODOS los campos, '1' sólo mapa de bits, '2' sólo array de inodos, '3' sólo el sizeOf estructural, '4' sólo los inodos ocupados.\n");
            exit(EXIT_FAILURE);
        }
    }
    bmount(argv[1]);
    bread(posSB,&sb);
    printf(MSG_INFO("leer_sf.c") " Analizando el contenido del fichero... \n");
    if(valorFiltro <= 0) { //Si es el -1 (predeterminado) o doy uso al 0. Leer el superbloque.
        separarFunciones();
        mostrarSuperbloque();
        separarFunciones();
    } 
    if(valorFiltro == 0 || valorFiltro == 1) { //Si uso el 0 o sólo ese; leer el mapa de bits (puede dar lugar a mostrar todo el contenido interno)
        separarFunciones();
        mostrarMapabits();
        separarFunciones();
    }
    if(valorFiltro == 0 || valorFiltro == 2) { //Si uso el 0 o sólo ese; leer el array de inodos (version simplificada)
        separarFunciones();
        mostrarArrayinodo();
        separarFunciones();
    }
    if(valorFiltro == 0 || valorFiltro == 3) { //Si uso el 0 o sólo ese; mostrar la distribución de y agrupación interna. (poco importante, pero sigue siendo parte de un test de prueba)
        separarFunciones();
        mostrarSizeofs();
        separarFunciones();
    }
    if(valorFiltro == 0 || valorFiltro == 4) { //Si uso el 0 o sólo ese; mostrar los inodos que esten en uso actualmente.
        separarFunciones();
        mostrarInodos();
        separarFunciones();
    }
    bumount();
    if(RETURN_DEV) printf(MSG_OK("leer_sf.c") "No hay más datos a procesar, finalizado.\n");
    return 0;
}

//Me ayuda a separar fronteras entre distintas ejecucciones.
int separarFunciones() {
    printf(MSG_INFO("******* ******* ******* ******* ") "\n");
    return 0;
}

//Mostrar los datos del superbloque. 0 o con -1 (ausencia de argumentos).
int mostrarSuperbloque() {
    if(MODO_DEV) printf(MSG_OK("leer_sf.c/mostrarSuperbloque") " initSB() <> resultados:\n");
    printf(MSG_VAR("SB.posPrimerBloqueMB") "%d\n", sb.posPrimerBloqueMB);
    printf(MSG_VAR("SB.posUltimoBloqueMB") "%d\n", sb.posUltimoBloqueMB);
    printf(MSG_VAR("SB.posPrimerBloqueAI") "%d\n", sb.posPrimerBloqueAI);
    printf(MSG_VAR("SB.posUltimoBloqueAI") "%d\n", sb.posUltimoBloqueAI);
    printf(MSG_VAR("SB.posPrimerBloqueDatos") "%d\n", sb.posPrimerBloqueDatos);
    printf(MSG_VAR("SB.posUltimoBloqueDatos") "%d\n", sb.posUltimoBloqueDatos);
    printf(MSG_VAR("SB.posInodoRaiz") "%d\n", sb.posInodoRaiz);
    printf(MSG_VAR("SB.posPrimerInodoLibre") "%d\n", sb.posPrimerInodoLibre);
    printf(MSG_VAR("SB.cantBloquesLibres") "%d\n", sb.cantBloquesLibres);
    printf(MSG_VAR("SB.cantInodosLibres") "%d\n", sb.cantInodosLibres);
    printf(MSG_VAR("SB.totBloques") "%d\n", sb.totBloques);
    printf(MSG_VAR("SB.totInodos") "%d\n", sb.totInodos);
    return 0;
}

//Mostrar internamente la reserva de los bloques (completa o simplificada acorde al BIT_DEV). 0 o con 1.
int mostrarMapabits() {
    if(MODO_DEV) printf(MSG_OK("leer_sf.c/mostrarMapabits") " initMB() <> resultados:\n");
    if(BIT_DEV) { //Inspeccionar profundamente el mapa de bits.
        for(int i = posSB; i < sb.totBloques; i++) {
            printf("[%d]: %hhu\t", i,leer_bit(i));
            //if(i % 8 == 7) printf("\n"); //Para hacer los saltos de linea.
        }
        printf("\n"); //Salto de linea restante.
    } else { //Version simplificada.
        printf(MSG_VAR("leerBit - SB") "bloque: %d, estado bit: %d\n", posSB, leer_bit(posSB));
        printf(MSG_VAR("leerBit - InicioMB") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueMB, leer_bit(sb.posPrimerBloqueMB));
        printf(MSG_VAR("leerBit - UltimoMB") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueMB, leer_bit(sb.posUltimoBloqueMB));
        printf(MSG_VAR("leerBit - InicioAI") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueAI, leer_bit(sb.posPrimerBloqueAI));
        printf(MSG_VAR("leerBit - UltimoAI") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueAI, leer_bit(sb.posUltimoBloqueAI));
        printf(MSG_VAR("leerBit - InicioDatos") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueDatos, leer_bit(sb.posPrimerBloqueDatos));
        printf(MSG_VAR("leerBit - UltimoDatos") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueDatos, leer_bit(sb.posUltimoBloqueDatos));
    }
    return 0;
}

//Mostrar el anexo de inodos y su agrupación por bloque. 0 o con 2.
int mostrarArrayinodo() {
    if(MODO_DEV) printf(MSG_OK("leer_sf.c/mostrarArrayinodo") " initAI() <> resultados:\n");
    struct inodo inodos[BLOCKSIZE/sizeof(struct inodo)];
    int siguienteInodo = sb.posPrimerInodoLibre;
    for(int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        bread(i,inodos);
        if(i < sb.posPrimerBloqueAI+3 || i > sb.posUltimoBloqueAI-3) {
            printf(MSG_NOTA("_TESTER/blocklook") "Bloque objetivo: %d [+%d]\n", i, i*BLOCKSIZE);
            if(i == sb.posPrimerBloqueAI+2) {
                printf(MSG_NOTA("_TESTER/blocklook") " [OMITIR FRANJA] .  .  . \n");
            }
        }
        for(int j=0; j < (BLOCKSIZE / INODOSIZE) && siguienteInodo < sb.totInodos; j++){ //BUGFIX: totInodos
            if(MODO_DEV && (siguienteInodo <= 10 || siguienteInodo > sb.totInodos-10)) {
                if(siguienteInodo == sb.posInodoRaiz) {
                    printf(MSG_VAR("inodo %d [mod %d], tipo '%c' -- RAÍZ") "next pointer: %d\n", siguienteInodo, j, inodos[j].tipo, inodos[j].punterosDirectos[0]);
                } else {
                    printf(MSG_VAR("inodo %d [mod %d], tipo '%c'") "next pointer: %d\n", siguienteInodo, j, inodos[j].tipo, inodos[j].punterosDirectos[0]);
                }
            }
            siguienteInodo++;
        }
    }
    return 0;
}

//Mostrar los cálculos de agrupación y longitudes. 0 o con 3.
int mostrarSizeofs() {
    if(MODO_DEV) printf(MSG_OK("leer_sf.c/mostrarSizeofs") " sizeof MISC <> resultados:\n");
    printf(MSG_VAR("info superbloque: sizeof") "%lu\n", sizeof(struct superbloque));
    printf(MSG_VAR("info inodo: blocksize") "%d\n", BLOCKSIZE);
    printf(MSG_VAR("info inodo: sizeof") "%lu\n", sizeof(struct inodo));
    printf(MSG_VAR("info: núm inodos por bloque [mod offset]") "%lu\n", BLOCKSIZE/sizeof(struct inodo));
    return 0;
}

//Mostrar los metadatos de todos los inodos ocupados.
int mostrarInodos() {
    if(MODO_DEV) printf(MSG_OK("leer_sf.c/mostrarInodos") " metadatos INODO <> resultados:\n");
    struct inodo actual;
    struct tm *ts; //Usado para la fecha.
    char atime[80]; //String para la fecha.
    char mtime[80]; //String para la fecha.
    char ctime[80]; //String para la fecha.
    DEFINIR_SELLO_RWX; //Importar del z_EXTRA la variable array "SELLO_RWX".
    int buscadores = sb.totInodos-sb.cantInodosLibres; //Conocer la cantidad de inodos ocupados, cuando esta cuenta llegue a cero (o ya lo sea) dejará de ser necesario seguir buscando porque fue el último.
    for(int i=0; i < sb.totInodos && buscadores > 0; i++) { //Mientras me falte algún inodo ocupado y siga dentro del rango del array... (puede funcionar incluso en inodos dispersos)
        leer_inodo(i,&actual); //Inspeccionar ese inodo su contenido de metadatos.
        if(actual.tipo != 'l') { //Aqui hay un inodo ocupado (lo que buscaba), revela lo que sepas de el.
            printf("\n");
            if(sb.posInodoRaiz == i) printf(MSG_AVISO("DATOS INODO %d --- RAIZ") ">>\n", i); //Casualmente, ese es la raíz (relevante para la capa de directorios)
            else printf(MSG_NOTA("DATOS INODO %d") "\n", i); //Inodo ordinario.
            printf(MSG_VAR("tipo:") "%c\n", actual.tipo); //El tipo de inodo. 'l' libre, 'f' fichero, 'd' directorio/carpeta
            if(actual.permisos >= 4 && actual.permisos <= 7) printf(MSG_VAR("permisos:") "'" TVAR(COLOR_VERDE, "%s") "' (%d)\n", SELLO_RWX[(int)actual.permisos], actual.permisos); //[PUEDA LEER] Tipo de permisos (hay que hacer el casting "char" a "int", no usar el atoi). +4 lectura, +2 escritura, +1 ejecucción.
            else if(actual.permisos > 0) printf(MSG_VAR("permisos:") "'" TVAR(COLOR_AMARILLO, "%s") "' (%d)\n", SELLO_RWX[(int)actual.permisos], actual.permisos); //[EDICION LIMITADA] Tipo de permisos (hay que hacer el casting "char" a "int", no usar el atoi). +4 lectura, +2 escritura, +1 ejecucción.
            else printf(MSG_VAR("permisos:") "'" TVAR(COLOR_CHILLON, "%s") "' (%d)\n", SELLO_RWX[0], actual.permisos); //[SIN ACCESO] Tipo de permisos (hay que hacer el casting "char" a "int", no usar el atoi). +4 lectura, +2 escritura, +1 ejecucción.
            ts = localtime(&actual.atime);
            strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("atime:") "%s\n", atime); //Fecha de acceso.
            ts = localtime(&actual.mtime);
            strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("mtime:") "%s\n", mtime); //Fecha de modificación.
            ts = localtime(&actual.ctime);
            strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
            printf(MSG_VAR("ctime:") "%s\n", ctime); //Fecha de creación.
            printf(MSG_VAR("nlinks:") "%d\n", actual.nlinks); //Cantidad de enlaces referidos sobre este inodo (cuantas referencias es anti-imagen hacia este inodo).
            printf(MSG_VAR("tamEnBytesLog:") "%d\n", actual.tamEnBytesLog); //Contenido máximo escrito (posible offset entre ellos).
            printf(MSG_VAR("numBloquesOcupados:") "%d\n", actual.numBloquesOcupados); //Cantidad de bloques de datos y de punteros esta dando uso.
            printf(MSG_VAR("punterosDirectos:") "[0] %d,  [1] %d,  [2] %d,  [3] %d,  [4] %d,  [5] %d,  [6] %d,  [7] %d,  [8] %d,  [9] %d,  [10] %d,  [11] %d\n", actual.punterosDirectos[0], actual.punterosDirectos[1], actual.punterosDirectos[2], actual.punterosDirectos[3], actual.punterosDirectos[4], actual.punterosDirectos[5], actual.punterosDirectos[6], actual.punterosDirectos[7], actual.punterosDirectos[8], actual.punterosDirectos[9], actual.punterosDirectos[10], actual.punterosDirectos[11]); //¿A que mola este trucho?, seguramente aún cabe en una linea en la terminal (o igual no...).
            printf(MSG_VAR("punterosIndirectos:") "[Ind0] %d,  [Ind1] %d,  [Ind2] %d,\n", actual.punterosIndirectos[0], actual.punterosIndirectos[1], actual.punterosIndirectos[2]);
            printf("\n");
            buscadores--; //Encontré un inodo ocupado, bajo el contador de inodos restantes.
        }
    }
    return 0;
}