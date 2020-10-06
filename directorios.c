#include "directorios.h"

struct UltimaEntrada UltimasEntradasLectura[BLOCKSIZE / sizeof(struct UltimaEntrada)];   //Caché global para lecturas
struct UltimaEntrada UltimasEntradasEscritura[BLOCKSIZE / sizeof(struct UltimaEntrada)]; //Caché global para escrituras
unsigned int UltimaPosLectura = 0;                                                       //Variable global para insertar una nueva entrada en la caché que se irá incrementando a cada inserción.
unsigned int UltimaPosEscritura = 0;                                                     //Lo mismo que la variable de arriba

//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                  FUNDAMENTAL
//
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    char *msg;
    char *cam = malloc(strlen(camino));

    unsigned int i = 0;

    if (*(camino) != '/') // Miramos si el camino que nos pasan empieza por '/'. Si no es así, retorna error.
    {
        printf(MSG_ERROR("directorios.c/extraer_camino") "Error de camino. Has introducido un formato de caminmo incorrecto.\n El camino introducido debe empezar por el carácter '/'.\n");
        return -1;
    }

    strcpy(cam, camino);                            //El problema de la asignacion sobre una constante.
    msg = (cam + 1);                                // Nos posicionamos en el primer carácter después de '/'
    while (*(msg + i) != '/' && *(msg + i) != '\0') // Copiamos los carácteres posteriores al primer '/' y anteriores al siguiente
    {
        *(inicial + i) = *(msg + i);
        i++;
    }

    if (*(msg + i) == '\0') // Si hemos llegado al final sin encontrar ningún '/' más, éste será un fichero y final = '\0'
    {
        strcpy(final, "\0");
        strcpy(tipo, "FICHERO");
    }
    else
    {
        strcpy(final, msg + i); // Si, por el contrario, encontramos otro /, significa que hay ficheros intermedios y que aún no sabemos si es un fichero o un directorio
        strcpy(tipo, "DIRECTORIO");
    }

    if (ENTRADA_DEV)
        fprintf(stderr, MSG_INFO("directorios.c/extraer_camino") "Inicial: %s. \n· Final: %s. \n· Tipo: %s.\n", inicial, final, tipo);

    return 0;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    char *inicial, *final, *tipo;
    struct inodo inodo_dir;
    struct entrada buff_lect[BLOCKSIZE / sizeof(struct entrada)];
    struct entrada entrada;
    unsigned int nentrada;
    int postRESERVAR = -1;                                     //(BUGFIX): Cuando se reserva un directorio, p_inodo_dir ha de modificarse a ese valor.
    int numentradas;                                           //Cantidad total de entradas contenidas dentro del inodo.
    int offsetEntradas = 0;                                    //Offset en caso de tener más entradas de las indicadas.
    int contadorInterno = 0;                                   //Usado para la lectura del buffer "buff_lect", le guia la celda del array.
    int amplitudEntradas = BLOCKSIZE / sizeof(struct entrada); //Para el while en hacer MOD y pedir siguiente bloque.

    //printf(TVAR(COLOR_CHILLON, "camino_Parcial '%s', p_inodo_dir '%d', p_inodo '%d', p_entrada '%d',reservar '%d',permisos '%d'") "\n",camino_parcial,*p_inodo_dir,*p_inodo,*p_entrada,reservar,permisos); //[PRUEBAS] Analisis inicial.

    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = 0;
        *p_entrada = 0;
        if (TRADUCIR_PINODOS)
            fprintf(stderr, MSG_OK("directorios.c/buscar_entrada") "Ruta '%s' padre traducida ⇒ Inodo padre: '%d', inodo fichero: '%d'\n", camino_parcial, *p_inodo_dir, *p_inodo);
        return 0;
    }

    inicial = malloc(BLOCKSIZE / 8);
    final = malloc(BLOCKSIZE / 8);
    tipo = malloc(BLOCKSIZE / 8);
    memset(inicial, 0, BLOCKSIZE / 8); //(BUGFIX): Experimenta problemas al extraer_camino por contener basura.
    memset(buff_lect, 0, BLOCKSIZE);   //(BUGFIX): El buffer no estaba limpio antes de leerlo.
    if (extraer_camino(camino_parcial, inicial, final, tipo) < 0)
    {
        return -1; //el mensaje de error será el mensaje de extraer_camino()
    }
    if (ENTRADA_DEV)
        fprintf(stderr, "Final: %s\n", final);
    //buscamos la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0)
    {
        return -1; //el mensaje de error será el mensaje de leer_inodo()
    }
    if (inodo_dir.tamEnBytesLog > 0)
        mi_read_f(*p_inodo_dir, buff_lect, 0, BLOCKSIZE); //Para suprimir el error, se ha de comprobar si tiene ALGO previamente.
    numentradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    if (ENTRADA_DEV)
        fprintf(stderr, MSG_VAR("directorios.c/buscar_entrada") "Numentradas: %d\n", numentradas);
    nentrada = 0;
    if (ENTRADA_DEV)
        fprintf(stderr, MSG_VAR("directorios.c/buscar_entrada") "Nombre_entrada: %s\n", buff_lect[nentrada].nombre);
    if (numentradas > 0)
    {
        //contadorInterno = 0; //Posiblemente no vaya a requerirlo...
        while ((nentrada < numentradas) && (strcmp(inicial, buff_lect[contadorInterno].nombre) != 0))
        {
            nentrada++;        //Cantidad total de entradas procesadas (LIMITE SUPERIOR).
            contadorInterno++; //Ubicación del array del buffer.
            if ((contadorInterno % amplitudEntradas == 0) && (nentrada < numentradas))
            {                                                                  //Cuando el número [0-15] llegue a valer 16, intercalar el bloque por otro.
                contadorInterno = 0;                                           //Redefinir la cuenta del array desde cero para carga otro bloque.
                memset(buff_lect, 0, BLOCKSIZE);                               //(BUGFIX) Mantener la entrada limpia.
                offsetEntradas += BLOCKSIZE;                                   //Pasar al siguiente bloque ocupado.
                mi_read_f(*p_inodo_dir, buff_lect, offsetEntradas, BLOCKSIZE); //Leer siguiente bloque y cargar los datos.
            }
        }
        if (nentrada < numentradas)
        { //BUGFIX: La variable "entrada" no llegaba a leer el contenido y a menudo no entraba en el if "nentrada == numentradas" y el p_inodo lo requiere.
            entrada.ninodo = buff_lect[contadorInterno].ninodo;
            strcpy(entrada.nombre, buff_lect[contadorInterno].nombre);
        }
    }
    if (nentrada == numentradas)
    {
        switch (reservar)
        {
        case '0':
            fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "No existe la entrada consultada \n");
            return -1;
            break;

        case '1':
            if (inodo_dir.tipo == 'f')
            {
                fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "No se ha podido crear la entrada del fichero (la ruta intermedia era un fichero)\n");
                return -1;
            }
            if (ENTRADA_DEV)
                fprintf(stderr, MSG_VAR("directorios.c/buscar_entrada") "Inodo.permisos: %u\n", inodo_dir.permisos);
            if ((inodo_dir.permisos & 2) != 2)
            {
                fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "No se dispone de permisos de escritura \n");
                return -1;
            }
            else
            {
                strcpy(entrada.nombre, inicial);
                if (ENTRADA_DEV)
                    fprintf(stderr, "Tipo (después de extraer camino): %s\n", tipo);
                if (strcmp(tipo, "DIRECTORIO") == 0)
                {
                    if (strcmp(final, "/") == 0)
                    {
                        //entrada.ninodo = reservar_inodo('d', 6);
                        entrada.ninodo = reservar_inodo('d', (int)permisos - 48); //Faltaba eso (crear con los permisos INDICADOS), no olvidar el casting.
                        if (ENTRADA_DEV)
                            fprintf(stderr, MSG_OK("directorios.c/buscar_entrada") "Reservado inodo %u (con el valor permiso %d)\n", entrada.ninodo, (int)permisos - 48);
                        postRESERVAR = entrada.ninodo; //(BUGFIX) Provocar que se dispare el reemplazo.
                    }
                    else
                    {
                        fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "No existe el directorio intermedio cuando se procesaba la parte '" TVAR(COLOR_CHILLON, "%s") "' \n",camino_parcial); //Aportaba poca información, mejor decir algo referido.
                        return -1;
                    }
                }
                else
                {
                    //entrada.ninodo = reservar_inodo('f', 6);
                    entrada.ninodo = reservar_inodo('f', (int)permisos - 48); //Faltaba eso (crear con los permisos INDICADOS), no olvidar el casting.
                    if (ENTRADA_DEV)
                        fprintf(stderr, MSG_OK("directorios.c/buscar_entrada") "Inodo (fichero) reservado : %u (con el valor permiso %d)\n", entrada.ninodo, (int)permisos - 48);
                }
                if (mi_write_f(*p_inodo_dir, &entrada, (nentrada) * sizeof(struct entrada), sizeof(struct entrada)) == -1)
                {
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                    }
                    fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "EXIT_FAILURE \n");
                    return -1;
                }
            } //esto sigue en error de escritura entonces
            break;
        }
    }
    if (*final == '\0')
    {
        if ((nentrada < numentradas) && (reservar == '1'))
        {
            fprintf(stderr, MSG_ERROR("directorios.c/buscar_entrada") "Entrada '" TVAR(COLOR_CHILLON, "%s") "' ya existente \n",camino_parcial); //Aportaba poca información, mejor decir algo referido.
            return -1;
        }
        *p_inodo = entrada.ninodo; //numero de inodo del directorio creado.
        *p_entrada = nentrada;     //nº entrada dentro del ultimo directorio que lo contiene

        if (TRADUCIR_PINODOS)
            fprintf(stderr, MSG_OK("directorios.c/buscar_entrada") "Ruta '%s' traducida ⇒ Inodo padre: '%d', inodo fichero: '%d'\n", camino_parcial, *p_inodo_dir, *p_inodo);
        return 0;
    }
    else
    {
        if (postRESERVAR >= 0)
        {                                  //(BUGFALL -- FIXED) Resuelve el fallo de la reserva del inodo DIRECTORIO, evitar buffer y usar el valor reservado.
            *p_inodo_dir = entrada.ninodo; //(BUGFIX) Devuelve carpetas de inodo inexistentes cuando a su vez son reservados.
        }
        else
        { //No se ha reservado ningun inodo DIRECTORIO, usar los valores del buffer.
            *p_inodo_dir = buff_lect[contadorInterno].ninodo;
        }
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
}

//Esta funcion actua como requerimiento de buscar_entrada, regula los errores comunes tanto desde la libreria como para los ejecutables.
//ANALIZA: "camino" y "permisos" (solo se contempla el ultimo caracter) sean valores validos, incluyendose el filtro "tipoinodo").
//Los primeros 6 argumentos son identicos al de buscar_entrada y se utilizaran de la misma manera.
//La ultima (7º), el 'tipoinodo' permite obligar a respetar la ruta como un fichero (1), directorio (2) o en aceptar ambas (0). Por ejemplo, mi_touch ha de usar el filtro 1 (impedir crear las carpetas), mi_mkdir ha de usar filtro 2 (impedir crear los ficheros), ch_mod le vale ambas y ha de definir el filtro 0.
//[ADVERTENCIA]: Esto 'no exime' de comprobar si lo que ha retornado sea un fichero o un directorio (sólo les afectarán aquellos que manejen entradas EXISTENTES).
int commonAPI_pinodo(const char *camino, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos, char tipoinodo)
{
    int longitudCamino = strlen(camino); //Extraer la longitud de la ruta insertada.

    if (longitudCamino > 0)
    { //Contiene una posible ruta (variable NO VACIA).
        if (camino[0] != '/')
        { //Verificar que empieza desde la RAIZ; si no es asi, abortar.
            fprintf(stderr, MSG_ERROR("directorios.c/commonAPI_pinodo") "'%s' ha de empezar con '/' (tiene que ser tratada como una ruta ABSOLUTA).\n", camino);
            return -1;
        }
        if (tipoinodo != '0')
        { //Si se exige un filtro de camino, entrar aqui; el filtro se encarga de comprobar que "camino" tenga como ultimo caracter "/" o no lo tenga. Si difieren, abortar.
            if (tipoinodo == '1' && camino[longitudCamino - 1] == '/')
            { //Me pide que sea fichero, si encuentra el '/' es un ERROR.
                fprintf(stderr, MSG_ERROR("directorios.c/commonAPI_pinodo") "'" TVAR(COLOR_CHILLON, "%s") "' ha de ser interpretado como un " TVAR(COLOR_AMARILLO, "FICHERO") ", quita el '/' que esta al final.\n", camino);
                return -1;
            }
            if (tipoinodo == '2' && camino[longitudCamino - 1] != '/')
            { //Me pide que sea directorio, la ausencia de '/' es un ERROR.
                fprintf(stderr, MSG_ERROR("directorios.c/commonAPI_pinodo") "'" TVAR(COLOR_CHILLON, "%s") "' ha de ser interpretado como un " TVAR(COLOR_CYAN, "DIRECTORIO") ", hay que agregar el '/' al final.\n", camino);
                return -1;
            }
        }
        if (permisos < 48 || permisos > 55)
        { //Compatibilidad ASCI, determinar si estan dentro del rango el valor de los permisos; esto se puede saber RESTANDOLE 48. (48 >> 0; 55 >> 7)
            fprintf(stderr, MSG_ERROR("directorios.c/commonAPI_pinodo") "El parametro <permisos> necesita un valor entero y este entre '0' hasta el '7' (incluidos).\n");
            return -1;
        }
        //Llegados a este punto, no se observa ningún error potencial.
        if (buscar_entrada(camino, p_inodo_dir, p_inodo, p_entrada, reservar, permisos) >= 0)
        { //Estado aceptador, le tiene que avisar que tipo es tratado.
            if (camino[longitudCamino - 1] == '/')
                return 2; //Avisar a la funcion que su objeto es un DIRECTORIO (p_inodo_dir).
            else
                return 1; //Avisar a la funcion que su objeto es un FICHERO (p_inodo).
        }
        else
        { //Buscar_entrada ha experimentado un error, anular. El mensaje de error ya se encarga el "buscar_entrada()".
            return -1;
        }
    }
    else
    { //Ruta VACIA.
        fprintf(stderr, MSG_ERROR("directorios.c/commonAPI_pinodo") "La ruta no fue declarada y esta no puede estar vacia.\n");
    }
    return -1;
}

//Analiza la cantidad de entradas que contiene el directorio.
int entradasContenidas(const char *camino)
{
    unsigned int p_inodo_dir = 0; //Ultimo directorio accedido.
    unsigned int p_inodo = 0;     //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;   //Donde se ubica en la celda de entradas.
    int retornoAPI = -2;          //Puesto que mi "tipoinodo" (commonAPI_pinodo) puede ser cualquiera ('0' en el ultimo argumento), he de reconocer su tipo (me lo indica el return del commonAPI_pinodo)
    struct inodo inodo;           //Hay que pedir al inodo la cantidad de entradas.
    retornoAPI = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '0'); //Anulo la directriz de directorios para aceptar cualquier formato para mi_ls.
    if (retornoAPI >= 0)
    {
        if (retornoAPI == 1)
        { //Fichero, variable de interes: p_inodo.
            leer_inodo(p_inodo, &inodo);
        }
        else if (retornoAPI == 2)
        { //Directorio, variable de interes: p_inodo_dir.
            leer_inodo(p_inodo_dir, &inodo);
        }
        if (inodo.tamEnBytesLog > 0 && inodo.tipo == 'd')
        { //Si tiene entradas y me aseguro que mi objetivo es un directorio.
            return inodo.tamEnBytesLog / sizeof(struct entrada);
        }
        else if(inodo.tipo == 'f') return 1; //Si es un fichero, indicar que sólo hay 1 entrada (para el malloc del 'mi_ls').
    }
    return 0;
}

//Permite traducir "camino" para poder cargar el padre del inodo (incluso directorios).
//Sólo será necesario si se requiere el inodo padre, como por ejemplo un cambio en las entradas.
//Ejemplo "/dir1/tosco/" retorna "/dir1/tosco", esto hace que el p_inodo_dir sea la referencia "dir1" y p_inodo "tosco".
//DESDE AFUERA, definir una variable "char *"; ejemplo: "//char *rutaPrimaria = padrePINODOdirAPI(camino);"
char* padrePINODOdirAPI(const char *camino) {
    int longitudCamino = strlen(camino); //Extraer la longitud de la ruta insertada.
    char *rutaPrimaria; //El camino que usaré para traducir las rutas.

    if(longitudCamino < 1 || camino[0] != '/') { //Si la ruta esta vacia o no empieza por "/", la función no puede hacer nada.
        fprintf(stderr, MSG_ERROR("directorios.c/padrePINODOdirAPI") "La ruta no puede estar vacia y ha de empezar por '/', abortar.\n");
        return "-"; //Retorna un caracter invalido que no es aceptador en la ruta.
    }
    if(camino[longitudCamino - 1] == '/' && longitudCamino > 1) { //Si me dan en formato directorio y no es la raíz, deshago la operación para obtener en formato FICHERO.
        rutaPrimaria = malloc(longitudCamino-1); //Reservar memoria para toda la cadena sin el / final (no es la raíz).
        memset(rutaPrimaria, 0 ,longitudCamino-1); //Duplicar el String, excluyendo el "/" final que no parte desde la raíz.
        memcpy(rutaPrimaria, camino, longitudCamino-1); //Del arreglo sometido.
    } else { //Y si ya esta como fichero o es la raíz...
        rutaPrimaria = malloc(longitudCamino); //Reservar memoria para toda la cadena.
        memset(rutaPrimaria, 0 ,longitudCamino); //Duplicar el String
        memcpy(rutaPrimaria, camino, longitudCamino); //Del arreglo sometido.
    }
    return rutaPrimaria; //Retorna el String de la ruta compatible con p_inodo_dir.
}



//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                   PROGRAMAS
//

//COMPLETO.
//Crear un fichero/directorio en la ruta contenida ('tipoCrear' ha de valer "1" para generar fichero o bien el valor "2" para generar un directorio).
int mi_creat(const char *camino, unsigned char permisos, char tipoCrear)
{
    unsigned int p_inodo_dir = 0; //Ultimo directorio accedido.
    unsigned int p_inodo = 0;     //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;   //Donde se ubica en la celda de entradas.
    int devolver;                 //Ya que no devolvemos 0 o  -1, debemos guardar el valor de commonAPI
    #ifdef ENTREGA_3
    mi_waitSem();
    #endif
    if((devolver = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '1', permisos, tipoCrear))<0){
        #ifdef ENTREGA_3
        mi_signalSem();
        #endif
        return -1;
    }
    #ifdef ENTREGA_3
    mi_signalSem();
    #endif
    return devolver;
}

//COMPLETO + AMPLIACIÓN.
//Guardar en el buffer del tipo "string", las entradas para imprimirlas por pantalla. Funciona tanto directorios como ficheros.
int mi_dir(const char *camino, char *buffer, unsigned char extended, unsigned char plusinodo)
{
    unsigned int p_entrada = 0, p_inodo = 0, p_inodo_dir = 0, offset = 0, pointer = 0, exclusivoFichero = 0;
    unsigned int SIZE = BLOCKSIZE / sizeof(struct entrada);
    int count = 0;//, ret = 0;
    struct entrada buffer_ent[SIZE];
    struct inodo inodo;
    struct tm *tm;
    char *rutaPrimaria = padrePINODOdirAPI(camino); //Obliga a ser tratado como una ruta fichero para tener el "inodo_dir" referido a su padre siempre, hay que tener en cuenta que "p_inodo" podria ser un directorio.
    char tmp[100];

    //ret = commonAPI_pinodo(rutaPrimaria, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '2'); //Restringir sólo a directorios.
    if(commonAPI_pinodo(rutaPrimaria, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '0') < 0) return -1; //Comprueba que la ruta sea válida sin importar si es directorio o fichero
    leer_inodo(p_inodo, &inodo);
    if(inodo.tipo == 'd') pointer = p_inodo; //La variable de interes (directorio) esta en la ruta misma.
    else {
        pointer = p_inodo_dir; //La variable de interes (directorio) esta en el padre, porque el objetivo es un fichero.
        exclusivoFichero = 1; //Activar los IF de anulación al procesar las entradas. El IF maneja el bool FALSE con 0, para el TRUE con 1.
    }

    //if (ret < 0) return -1; //Anteriormente con sólo directorios...
    //pointer = p_inodo_dir; //Anteriormente con sólo directorios...

    leer_inodo(pointer, &inodo);
    if (ENTRADA_DEV) printf(MSG_VAR("directorios.c/mi_dir") "Numero de entradas en directorio: %ld\n", inodo.tamEnBytesLog / sizeof(struct entrada));

    if (inodo.tamEnBytesLog <= 0) return -1;

    if (inodo.tipo != 'd')
    {
        fprintf(stderr, MSG_ERROR("directorios.c/mi_dir") "Error: el inodo que intentas leer no es un directorio. \n");
        return -1;
    }

    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, MSG_ERROR("directorios.c/mi_dir") "Error: el directorio no tiene permisos de lectura. \n");
        return -1;
    }

    DEFINIR_SELLO_RWX;
    // Antes de leer el buffer, lo limpiaremos, porque hay basura
    memset(buffer_ent, 0, BLOCKSIZE);

    while (mi_read_f(pointer, buffer_ent, offset, BLOCKSIZE) > 0)
    {
        if(exclusivoFichero) mi_read_f(pointer, &buffer_ent, p_entrada * sizeof(struct entrada), sizeof(struct entrada)); //[OVERRIDE] Reemplaza los datos del buffer por el del fichero exclusivo; esto hará que se guarde en la buffer_ent[0] la entrada del fichero.
        // Esta es la versión simple. Solo concatenamos el nombre de cada entrada y un separador
        for (unsigned int i = 0; is_null_entry(buffer_ent[i]) && i < SIZE; i++)
        {
            count++; //(FIX), cantidad de entradas VALIDAS.
            leer_inodo(buffer_ent[i].ninodo, &inodo); //(FIX): Lo va a requerir con o sin extended para color de nombre.
            //printf("ELEM PROCESADO '%d'\tnombre '%s'\t ninodo '%d'\n",count,buffer_ent[i].nombre,buffer_ent[i].ninodo); //[PRUEBAS] para ver que ha cogido en cada paso.
            if (plusinodo)
            {
                sprintf(tmp, "%d\t", buffer_ent[i].ninodo);
                strcat(buffer, tmp);
            }
            if (extended == 1)
            {
                if (inodo.tipo == 'd')
                    strcat(buffer, "d"); //Como directorio.
                else
                    strcat(buffer, "-");                   //Caso de fichero.
                strcat(buffer, SELLO_RWX[inodo.permisos]); //Sellos de permisos
                strcat(buffer, "\t\t");                    //Tabulación de los permisos.
                tm = localtime(&inodo.mtime);              //Formato de tiempo.
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t", tm->tm_year + 1900,
                        tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);                     //Tiempo.
                sprintf(tmp, "%d", inodo.tamEnBytesLog); //Tamaño contenido.
                strcat(buffer, tmp);                     //TamBytes.
                strcat(buffer, "\t\t");                  //Tabulación del tamaño del inodo.
            }

            //La base de mostrar los objetos. Los nombres se colorean si son "ficheros ejecutables" o "directorios" (si extended esta activo)
            if (inodo.tipo == 'd' && extended != 0)
            { //Directorio.
                sprintf(tmp, "" TVAR(COLOR_AZUL, "%s") "", buffer_ent[i].nombre);
                strcat(buffer, tmp);
            }
            else if (inodo.tipo == 'f' && (inodo.permisos & 1) == 1 && extended != 0)
            { //Archivo ejecutable.
                sprintf(tmp, "" TVAR(COLOR_VERDE, "%s") "", buffer_ent[i].nombre);
                strcat(buffer, tmp);
            }
            else
            { //Sin relevancia de color.
                strcat(buffer, buffer_ent[i].nombre);
            }
            strcat(buffer, "\n"); //Salto de linea.
            if(exclusivoFichero) return 1; //[OVERRIDE] Finaliza prematuramente por ser un fichero; el texto ya se ha guardado.
        }
        offset += BLOCKSIZE;
        memset(buffer_ent, 0, BLOCKSIZE); //Limpiar para prevenir entradas repetidas
    }
    if (count <= 0)
        fprintf(stderr, MSG_AVISO("directorios.c/mi_dir") "El inodo '%d' no contiene ninguna entrada.\n", pointer);
    return count;
}

//[AUXILIAR] Comprueba si existe alguna entrada (retorna 1 [TRUE]; 0 [FALSE]). [reparado y simplificado]
int is_null_entry(struct entrada entry)
{
    if (strcmp(entry.nombre, "") != 0)
        return 1; //Si diferen en el strcmp (> 0), es una entrada NO VACIA.
    return 0;     // "" es igual a "", le dará valor 0
}

//[AUXILIAR] Comprueba que el camino pasado por parámetro esté dentro de la caché de lexctura/escritura
//Si rnw = 0 entonces es escritura, si rnw = 1 entonces es lectura
//[RETURN] La función retorna 1 si encuentra el camino en la caché de lectura, 2 si lo encuentra en la de escritura y 0 si no está en la caché
//Retorna en *point el puntero de la última entrada que ha encontrado, siempre que la haya encontrado.
int is_in_cache(const char *camino, unsigned char rnw, unsigned int *point)
{
    int found = 0;

    for (unsigned int i = 0; (i < (BLOCKSIZE / sizeof(struct UltimaEntrada))) && (found != 1); i++)
    {
        if (rnw == 1) //Buscamos en la caché de entradas de lectura
        {
            if (strcmp(UltimasEntradasLectura[i].camino, camino) == 0)
            {
                found = 1;
                *point = UltimasEntradasLectura[i].p_inodo;
                if (CACHE_DEV)
                    fprintf(stderr, MSG_INFO("directorios.c/is_in_cache : lectura") "Encontrada la entrada %s con inodo %d.\n", UltimasEntradasLectura[i].camino, UltimasEntradasLectura[i].p_inodo);
            }
        }
        else if (rnw == 0) //Buscamos en la caché de entradas de escritura
        {
            if (strcmp(UltimasEntradasEscritura[i].camino, camino) == 0)
            {
                found = 1;
                *point = UltimasEntradasEscritura[i].p_inodo;
                if (CACHE_DEV)
                    fprintf(stderr, MSG_INFO("directorios.c/is_in_cache : escritura") "Encontrada la entrada %s con inodo %d.\n", UltimasEntradasEscritura[i].camino, UltimasEntradasEscritura[i].p_inodo);
            }
        }
    }

    return found;
}

//COMPLETO
//Extrae los metadatos del inodo, además como extra de retornar el número del inodo para futuras referencias.
int mi_stat(const char *camino, struct STAT *p_stat, int *ninodo)
{
    unsigned int p_inodo_dir = 0; //Ultimo directorio accedido.
    unsigned int p_inodo = 0;     //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;   //Donde se ubica en la celda de entradas.
    int retornoAPI = -2;          //Debido que puede ser ficheros o directorios, he de reconocer cual es.

    //Si es "aceptado" la entrada, mostrar su contenido.
    retornoAPI = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '0');
    if (retornoAPI >= 0)
    {
        if (retornoAPI == 1)
        { //Fichero, variable de interes: p_inodo.
            *ninodo = p_inodo;
            return mi_stat_f(p_inodo, p_stat);
        }
        else if (retornoAPI == 2)
        { //Directorio, variable de interes: p_inodo_dir.
            *ninodo = p_inodo_dir;
            return mi_stat_f(p_inodo_dir, p_stat);
        }
    }
    return -1;
}

//COMPLETO.
//Cambia los permisos del inodo (se admite directorios y ficheros EXISTENTES)
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0;                                                                  //Ultimo directorio accedido.
    unsigned int p_inodo = 0;                                                                      //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;                                                                    //Donde se ubica en la celda de entradas.
    int retornoAPI = -2;                                                                           //Puesto que mi "tipoinodo" (commonAPI_pinodo) puede ser cualquiera ('0' en el ultimo argumento), he de reconocer su tipo (me lo indica el return del commonAPI_pinodo)
    retornoAPI = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', permisos, '0'); //Acepta tanto fichero como carpeta, he de regular que tipo me va a retornar.
    if (retornoAPI >= 0)
    {
        if (retornoAPI == 1)
        { //Fichero, variable de interes: p_inodo.
            return mi_chmod_f(p_inodo, permisos);
        }
        else if (retornoAPI == 2)
        { //Directorio, variable de interes: p_inodo_dir.
            return mi_chmod_f(p_inodo_dir, permisos);
        }
    }
    return -1; //Cualquier otro error no aceptable...
}

//COMPLETO
//Leer el contenido del fichero en la ruta especificada, con posibilidad de dejar un margen (que puede omitirse).
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;           //Necesario para ver su tipo
    unsigned int p_inodo_dir = 0; //Ultimo directorio accedido.
    unsigned int p_inodo = 0;     //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;   //Donde se ubica en la celda de entradas.
    unsigned int pointer = 0;
    int returnLIB = -2; //Debido que puede ser ficheros o directorios, he de reconocer cual es.

    if (is_in_cache(camino, 1, &pointer) == 0)
    {                                                                                            //Buscamos el camino en la caché de escrituras; si esta presente se guardará en la variable "pointer".
        returnLIB = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '0'); //Sólo se puede leer en "ficheros".
        if (returnLIB >= 0)
        {
            pointer = p_inodo; //Puesto que mi filtro he especificado "ficheros", de por seguro, es "p_inodo".
            //En caso de no haber encontrado la entrada, la añadimos a la caché de entradas de lectura
            UltimasEntradasLectura[UltimaPosLectura].p_inodo = pointer;
            strcpy(UltimasEntradasLectura[UltimaPosLectura].camino, camino);
            UltimaPosLectura = (UltimaPosLectura + 1) % (BLOCKSIZE / sizeof(struct UltimaEntrada)); //La última posición se actualizará de forma cíclica
        }
        else
        {
            return -1;
        }
    }
    leer_inodo(pointer, &inodo);
    if (inodo.tipo != 'f')
    {
        fprintf(stderr, MSG_ERROR("directorios.c/mi_read") "El inodo %d no es un fichero, imposible de leer.\n", pointer);
        return -1;
    }
    //El semáforo se parará en las zonas de mi_read_f que sean necesarias
    return mi_read_f(pointer, buf, offset, nbytes);
}

//COMPLETO
//Escribe en el fichero un texto a la ruta especificada, con posibilidad de dejar un margen.
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;           //Necesario para ver su tipo
    unsigned int p_inodo_dir = 0; //Ultimo directorio accedido.
    unsigned int p_inodo = 0;     //Fichero apuntado (si tuviera).
    unsigned int p_entrada = 0;   //Donde se ubica en la celda de entradas.
    unsigned int pointer = 0;
    int returnLIB = -2; //Debido que puede ser ficheros o directorios, he de reconocer cual es.
    if (is_in_cache(camino, 0, &pointer) == 0)
    {                                                                                            //Buscamos el camino en la caché de escrituras; si esta presente se guardará en la variable "pointer".
        //No hace falta parar el semaforo ya que no reserva
        returnLIB = commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '1'); //Sólo se puede escribir en "ficheros".
        if (returnLIB >= 0)
        {
            pointer = p_inodo; //Puesto que mi filtro he especificado "ficheros", de por seguro, es "p_inodo".
            //En caso de no haber encontrado la entrada, la añadimos a la caché de entradas de lectura
            UltimasEntradasEscritura[UltimaPosEscritura].p_inodo = pointer;
            strcpy(UltimasEntradasEscritura[UltimaPosEscritura].camino, camino);
            UltimaPosEscritura = (UltimaPosEscritura + 1) % (BLOCKSIZE / sizeof(struct UltimaEntrada)); //La última posición se actualizará de forma cíclica
        }
        else
        {
            return -1;
        }
    }
    //En el caso en el que hayamos encontrado el camino en la caché, tendremos en pointer el p_inodo de la entrada en concreto.
    //Ahora hay que inspeccionar que mi inodo sea el tipo "fichero" (no se me haya colado un directorio). El permiso y las otras regulaciones ya se encarga el "write_f"
    leer_inodo(pointer, &inodo);
    if (inodo.tipo != 'f')
    {
        fprintf(stderr, MSG_ERROR("directorios.c/mi_write") "El inodo %d no es un fichero, imposible de escribir.\n", pointer);
        return -1;
    }
    //Dentro de mi_write_f se controla el semáforo así que no hace falta controlarlo aqui
    return mi_write_f(pointer, buf, offset, nbytes);
}

// COMPLETO.
int mi_link(const char *camino1, const char *camino2)
{
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0, p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;
    struct inodo inodo1;
    struct inodo inodo2;
    struct entrada buffer; 
    //No hace falta parar el semaforo ya que commonAPI no reserva
    if (commonAPI_pinodo(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, '0', '0', '1') != -1)
    {
        //Comprobar p_inodo1 permiso de lectura
        leer_inodo(p_inodo1, &inodo1);
        if ((inodo1.permisos & 4) == 4)
        {
            if(inodo1.tipo != 'f') {
                fprintf(stderr, MSG_ERROR("directorios.c/mi_link") "La ruta de origen '" TVAR(COLOR_CHILLON, "%s") "' no es un inodo fichero.\n",camino1);
                return -1;
            }
            #ifdef ENTREGA_3
            mi_waitSem();   //commonAPI puede tenerse como zona critica
            #endif
            if (commonAPI_pinodo(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, '1', '6', '1') != -1)
            {
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
                leer_inodo(p_inodo2, &inodo2);             
                mi_read_f(p_inodo_dir2, &buffer, p_entrada2*sizeof(struct entrada), sizeof(struct entrada));  //BUGFIX: Recoge la entrada, necesita un bloque entero para no generar errores.

                buffer.ninodo = p_inodo1; //Creamos el enlace: asociamos la entrada de p_inodo 1 a la entrada de camino2
                mi_write_f(p_inodo_dir2, &buffer, p_entrada2*sizeof(struct entrada), sizeof(struct entrada));
                #ifdef ENTREGA_3
                mi_waitSem();       //Paramos para liberar inodo, actualizar ctime y nlinks
                #endif
                liberar_inodo(p_inodo2);
                leer_inodo(p_inodo1, &inodo1); //Leer los campos y actualizarlos
                inodo1.nlinks++;
                inodo1.ctime = time(NULL);
                escribir_inodo(p_inodo1, inodo1);  //Y faltaba tambien escribirlo para que se guardaran los cambios
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
            }
            else
            {
                fprintf(stderr, MSG_ERROR("directorios.c/mi_link") "Camino 2 no se ha podido crear o es un directorio!\n");
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
                return -1;
            }
        }
        else
        {
            fprintf(stderr, MSG_ERROR("directorios.c/mi_link") "No hay permisos de lectura para inodo1.\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, MSG_ERROR("directorios.c/mi_link") "Error: fichero de camino1 no existente, no encontrado o es un directorio.\n");
        return -1;
    }
    return 0;
}


//COMPLETO
//Reubica el contenido de la ruta de origen a una de destino (sin nombres repetidos).
int mi_move(const char* origen, const char* destino) {
    unsigned int p_inodo_dirORI = 0, p_inodoORI = 0, p_entradaORI = 0; //Referencia del origen.
    unsigned int p_inodo_dirDST = 0, p_inodoDST = 0, p_entradaDST = 0; //Referencia de la nueva.
    int permisos; //Necesario para el casting de permisos y reconvertirlo en ASCI.
    struct inodo inodoDESPLAZAR; //Inodo objetivo para retirar de la ruta inicial.
    struct inodo PadresInodo; //Referido al padre de las rutas (usado para revisar permisos de escritura).
    struct entrada origenENT; //Objeto entrada para extraer y retirarlo.
    struct entrada salidaENT; //Objeto entrada que se clona para insertar.
    char *rutaOrigen = padrePINODOdirAPI(origen);
    char *generarRuta = padrePINODOdirAPI(destino);
    if(commonAPI_pinodo(rutaOrigen, &p_inodo_dirORI, &p_inodoORI, &p_entradaORI, '0', '0', '0') >= 0) { //El tipo es irrelevante y siempre permanece en formato fichero para p_inodo_dir y p_inodo alineados.
        leer_inodo(p_inodo_dirORI, &PadresInodo); //Recoger los datos, siempre lo tendré alojado en inodoDir por exigir formato de API.
        if ((PadresInodo.permisos & 2) != 2) { //Revisa que desde el directorio padre del origen tenga permisos de escritura porque cambiará la estructura de los datos.
            fprintf(stderr, MSG_ERROR("directorios.c/mi_dir") "El directorio que contiene '" TVAR(COLOR_CHILLON, "%s") "' no tiene permisos de escritura, imposible de mover. \n",rutaOrigen);
            return -1;
        }
        leer_inodo(p_inodoORI, &inodoDESPLAZAR); //Leer el contenido del inodo de origen, necesario para emitir los permisos.
        permisos = ((int)inodoDESPLAZAR.permisos)+48; //Casting a INT, luego usar la nomeclatura de ASCI (agregar +48).
        if(commonAPI_pinodo(generarRuta, &p_inodo_dirDST, &p_inodoDST, &p_entradaDST, '1', permisos, '0') >= 0) { //Provoca que se reserve el inodo objetivo, si acepta, ya habré generado uno.
            //EXPLICACIÓN: Con mi_link sólo funciona en ficheros y el unlink quitaria todos los objetos contenidos del directorio.
            //¿SOLUCIÓN? Generar la ruta de destino e intercambiar los inodos de las rutas, luego unlink a la ruta de origen (inodo nuevo).
            #ifdef ENTREGA_3
            mi_waitSem();   //Paramos para intercambiar las referencias de inodo en las entradas de directorio.
            #endif
            mi_read_f(p_inodo_dirORI, &origenENT, p_entradaORI * sizeof(struct entrada), sizeof(struct entrada)); //Extraer la entrada del origen.
            mi_read_f(p_inodo_dirDST, &salidaENT, p_entradaDST * sizeof(struct entrada), sizeof(struct entrada)); //Extraer la entrada nueva.
            salidaENT.ninodo = p_inodoORI; //Asigna la nueva entrada la referencia como el inodo original.
            origenENT.ninodo = p_inodoDST; //Cambia el inodo de origen por la nueva (y después unlink).
            mi_write_f(p_inodo_dirORI, &origenENT, p_entradaORI * sizeof(struct entrada), sizeof(struct entrada)); //Solapar el origen, insertandole el inodo nuevo para suprimirlo.
            mi_write_f(p_inodo_dirDST, &salidaENT, p_entradaDST * sizeof(struct entrada), sizeof(struct entrada)); //Solapar la entrada de destino con la referencia del original (para mantenerlo sin usar nlinks).
            #ifdef ENTREGA_3
            mi_signalSem(); //Continuar a partir de este punto.
            #endif
            return mi_unlink(rutaOrigen,0); //Elimina la ruta original (le puse el inodo nuevo vacio) y lo quita de las entrada. No he suprimido el original porque esta ahora en la ruta del destino.
        }
    }
    return -1;
}


//COMPLETO
//Version minimalista de eliminar exclusivamente directorios, compatible con "mi_rmdir" y "mi_rmdir_r" por via "unlink".
int mi_rmdir(const char *camino, unsigned char recursivo) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0; //Las tres de siempre para buscar entrada.
    struct inodo inodo; //Inodo objetivo para eliminar, asumiendo que sea un directorio.
    if(commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '2') >= 0) { //Admite sólo DIRECTORIOS, este filtro siempre retorna la variable de interes en p_inodoDIR.
        leer_inodo(p_inodo_dir, &inodo); //Recoger los datos, siempre lo tendré alojado en inodoDir por exigir el modo 2 (ruta que acaba en /).
        if(inodo.tipo == 'd') { //Sólo acepta DIRECTORIOS, si se encuentra un fichero ha de finalizar prematuramente.
            return mi_unlink(camino,recursivo); //Delega su tarea al unlink para eliminar el directorio y su estado recursivo.
        }
        //Si sale, es porque no es aceptador y seguramente se me habrá colado un fichero confundiendo el / final.
        fprintf(stderr, MSG_ERROR("directorios.c/mi_rmdir") "La ruta '" TVAR(COLOR_CHILLON, "%s") "' no es un directorio, use el mi_rm en su lugar.\n",camino);
    }
    return -1;
}


//COMPLETO
//Usado por el ejecutable unlink, comprueba si la ruta es un fichero de ser así, tratará de retirar el enlace o incluso suprimirlo si se autoriza.
int mi_unlink_f(const char *camino, unsigned char suprimible) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0; //Las tres de siempre para buscar entrada.
    struct inodo inodo; //Inodo objetivo para eliminar, asumiendo que sea un fichero.
    if (commonAPI_pinodo(camino, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '1') >= 0) {
        leer_inodo(p_inodo_dir, &inodo); //Leer el directorio para ver sus permisos.
        if ((inodo.permisos & 2) == 2) { //Necesita que su directorio se pueda escribir.
            leer_inodo(p_inodo, &inodo); //Leer el inodo objetivo para ver si es un fichero y su número de enlaces.
            if(inodo.tipo == 'f') { //Me aseguro que sea un fichero, mi_link no permite procesar directorios.
                if(inodo.nlinks <= 1 && suprimible != 1) { //Si ve el caso de 1 nlink sin habilitar el suprimir, denegar. Sólo con el modo suprimir puede hacer objetivo a inodos fichero de 1 enlace.
                    fprintf(stderr, MSG_ERROR("directorios.c/mi_unlink_f") "El inodo sólo usa 1 enlace, agrege el argumento -f si quiere suprimirlo.\n");
                    return -1;
                }
                return mi_unlink(camino,0); //Resto de casos, con nlink >=2 o con el suprimir activo en el fichero.
            } else { //El inodo no es un fichero, no esta permitido seguir.
                fprintf(stderr, MSG_ERROR("directorios.c/mi_unlink_f") "La ruta de origen '" TVAR(COLOR_CHILLON, "%s") "' no es un inodo fichero.\n",camino);
            }
        } else { //El directorio al que hace referencia no tiene permiso de escritura.
            fprintf(stderr, MSG_ERROR("directorios.c/mi_unlink_f") "El directorio no tiene permisos de escritura, imposible de alterar su estructura.\n");
        }
    } //commonAPI ya mostrará el error pertinente.
    return -1;
}


//COMPLETO
//Anula la(s) entrada(s) de la ruta especificada, si la recursividad esta activa y es un directorio, todo su contenido se verá afectado.
int mi_unlink(const char *camino, unsigned char recursivo) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    unsigned int BloqueEntradasSIZE = BLOCKSIZE / sizeof(struct entrada); //Determina la asignación de entradas en un bloque.
    int nentradas = 0; //Cantidad de entradas que tiene el inodo padre.
    struct inodo inodo; //Inodo objetivo para eliminar.
    struct inodo supinodo; //El inodo padre del objetivo.
    struct entrada buff_lect; //Objeto entrada que se clona para insertar.
    struct entrada buffer[BloqueEntradasSIZE]; //Previene el fallo del BLOCKSIZE <1024 en la lectura.
    char *rutaPrimaria = padrePINODOdirAPI(camino); //Obliga a ser tratado como una ruta fichero para tener el "inodo_dir" referido a su padre siempre, hay que tener en cuenta que "p_inodo" podria ser un directorio.
    char *caminopar = malloc(BLOCKSIZE / 4); //Requerido para la recursividad.
    memset(caminopar, 0, BLOCKSIZE / 4); //Prelimpieza del recursivo.
    memset(buffer, 0, BLOCKSIZE); //Prelimpieza del buffer de entradas BLOCKSIZE.

    if(commonAPI_pinodo(rutaPrimaria, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '0') >= 0) { //El tipo es irrelevante y siempre permanece en formato fichero para p_inodo_dir y p_inodo alineados.
        leer_inodo(p_inodo, &inodo); //Leer el inodo objetivo para el unlink...
        if(inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) { //Si el objetivo es un directorio NO VACIO.
            if(recursivo == 0) { //En modo normal, no se puede.
                fprintf(stderr, MSG_ERROR("directorios.c/mi_unlink") "El directorio '" TVAR(COLOR_CHILLON, "%s/") "' no esta vacio y no se puede borrar.\n",rutaPrimaria);
                return -1;
            } else { // [RECURSIVO] Borrar todos los elementos contenidos dentro de la carpeta al exigir la recursiva. Va borrando 1 a 1 y es fiable este sistema.
                nentradas = inodo.tamEnBytesLog / sizeof(struct entrada); //Contar la cantidad de entradas del inodo OBJETIVO.
                for (int i = nentradas-1; i >= 0; i--) { //El unlink tiene mejor complejidad y tiempo desde entradas finales, hay que invertir el proceso; NADA DE UNSIGNED que ya te veo venir.
                    mi_read_f(p_inodo, &buffer, i * sizeof(struct entrada), sizeof(struct entrada));  //BUGFIX: Para recoger la entrada sin errores necesita un bloque entero, a pesar de que se escriba sólo 1. Coger la última entrada del OBJETIVO.
                    strcpy(caminopar, rutaPrimaria); //Clonar la ruta original.
                    strcat(caminopar, "/"); //Le pongo la barra suprimida
                    strcat(caminopar, buffer[0].nombre); //Y transferir el nombre del objeto.
                    mi_unlink(caminopar, 1);
                }
            }
        }
        leer_inodo(p_inodo_dir, &supinodo); //Leer el inodo padre.
        nentradas = supinodo.tamEnBytesLog / sizeof(struct entrada); //Contar la cantidad de entradas del inodo padre.
        if(p_entrada != nentradas - 1) {
            mi_read_f(p_inodo_dir, &buffer, (supinodo.tamEnBytesLog - sizeof(struct entrada)), sizeof(struct entrada));
            buff_lect = buffer[0]; //BUGFIX: Extrae la unica entrada.
            mi_write_f(p_inodo_dir, &buff_lect, (p_entrada * sizeof(struct entrada)), sizeof(struct entrada));
        }
        #ifdef ENTREGA_3
        mi_waitSem();   //Paramos para leer inodo, cambiar nlinks, liberar inodo, cambiar ctime y escribir inodo
        #endif
        mi_truncar_f(p_inodo_dir, (supinodo.tamEnBytesLog - sizeof(struct entrada))); //Indistintamente de los pasos, borrar la última entrada.
        leer_inodo(p_inodo, &inodo);
        inodo.nlinks--;
        if (inodo.nlinks <= 0) {
            if(ENTRADA_DEV) fprintf(stderr, MSG_INFO("directorios.c/mi_unlink") "Inodo %d con 0 nlinks húerfano, retirado.\n",p_inodo);
            liberar_inodo(p_inodo); //Si al detraer era el último, retirar el inodo y reciclar.
        }
        else {
            inodo.ctime = time(NULL); //Reinicia la marca de tiempo.
            escribir_inodo(p_inodo, inodo); //Actualiza la marca del tiempo y nlinks nuevo.
        }
        #ifdef ENTREGA_3
        mi_signalSem(); //Continuar a partir de este punto.
        #endif
        return 0;
    }
    return -1;
}


//COMPLETO
//Version minimalista de copiar exclusivamente un único fichero (asumiendo que tenga los permisos de lectura y escritura), compatible con "mi_cp_f" por via "mi_copy".
int mi_copy_f(const char* origen, const char* destino) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0; //Las tres de siempre para buscar entrada.
    struct inodo inodo; //Inodo objetivo para copiar, asumiendo que sea un fichero.
    if(commonAPI_pinodo(origen, &p_inodo_dir, &p_inodo, &p_entrada, '0', '0', '1') >= 0) { //Admite sólo FICHEROS, este filtro siempre retorna la variable de interes en p_inodo.
        leer_inodo(p_inodo, &inodo); //Recoger los datos, siempre lo tendré alojado en p_inodo por exigir el modo 1 (ruta que no puede acabar con /).
        if(inodo.tipo == 'f') { //Sólo acepta FICHEROS, si se encuentra un directorio ha de finalizar prematuramente.
            if((inodo.permisos & 4) != 4 || (inodo.permisos & 2) != 2) {
                fprintf(stderr, MSG_ERROR("directorios.c/mi_copy_f") "La ruta '" TVAR(COLOR_CHILLON, "%s") "' carece de permiso de lectura y escritura, generaria un fichero vacio.\n",origen);
                return -1;
            }
            return mi_copy(origen,destino); //Delega su tarea al copy para copiar el fichero.
        }
        //Si sale, es porque no es aceptador y seguramente se me habrá colado un directorio confundiendo el / final.
        fprintf(stderr, MSG_ERROR("directorios.c/mi_copy_f") "La ruta '" TVAR(COLOR_CHILLON, "%s") "' no es un fichero, use mi_cp en su lugar.\n",origen);
    }
    return -1;
}


//COMPLETO
//Tratar de duplicar como objetos diferentes toda la estructura de la ruta original, respetando el marco de permisos; funciona con todas las rutas y tipos.
int mi_copy(const char* origen, const char* destino)
{
    unsigned int p_inodo_dirORI = 0, p_inodoORI = 0, p_entradaORI = 0; //Referencias del buscar_entrada para la ruta origen.
    unsigned int p_inodo_dirDST = 0, p_inodoDST = 0, p_entradaDST = 0; //IDEM anterior para la ruta destino.
    int permisos; //Necesario para el casting de permisos y reconvertirlo en ASCI.
    int nentradas = 0; //Cantidad de entradas que tiene el inodo de origen. Reciclada para contar el offset interno para el offset del buffer referido a un bloque.
    int bytesFichero = 0; //Conocer la amplitud.
    int offsetFichero = 0; //Offset acumulativo a media que va leyendo el fichero.
    struct inodo inodoORI; //Inodo donde se analiza sus datos.
    struct entrada proximaEntrada; //Objeto entrada que se clona para insertar.
    unsigned char vacio[BLOCKSIZE]; //[CONSTANTE] Usado para el memcmp como un bloque vacio para el buffer de ficheros.
    char bufferFichero[BLOCKSIZE]; //Usado para replicar los datos del fichero.
    char *rutaOrigen = padrePINODOdirAPI(origen);
    char *generarRuta = padrePINODOdirAPI(destino);
    char *nuevoOrigen = malloc(BLOCKSIZE / 4); //Requerido para la recursividad, anexa la ruta a la nueva sin solaparla.
    char *nuevoDestino = malloc(BLOCKSIZE / 4); //Requerido para la recursividad, anexa la ruta a la nueva sin solaparla.
    memset(nuevoOrigen, 0, BLOCKSIZE / 4); //Prelimpieza del recursivo.
    memset(nuevoDestino, 0, BLOCKSIZE / 4); //Prelimpieza del recursivo.
    memset(vacio, 0, BLOCKSIZE); //[CONSTANTE] Para identificar si un bloque sigue vacio o tenga algo.
    memset(bufferFichero, 0, BLOCKSIZE); //Prelimpieza para la lectura del fichero.

    if(commonAPI_pinodo(rutaOrigen, &p_inodo_dirORI, &p_inodoORI, &p_entradaORI, '0', '0', '0') >= 0) {
        leer_inodo(p_inodoORI, &inodoORI); //Leer el inodo de origen.
        if(inodoORI.tipo == 'd') { //[PRE-FASE] Si este es un directorio, hay que concatenar rutas y prepararlo para la recursiva.
            strcat(rutaOrigen, "/"); //Le pongo la barra suprimida.
            strcat(generarRuta, "/"); //Le pongo la barra suprimida; esto provoca que se reserve un tipo de directorio.
        }
        permisos = ((int)inodoORI.permisos)+48; //Casting a INT, luego usar la nomeclatura de ASCI (agregar +48).
        if(commonAPI_pinodo(generarRuta, &p_inodo_dirDST, &p_inodoDST, &p_entradaDST, '1', permisos, '0') >= 0) {
            if((inodoORI.permisos & 4) != 4 || (inodoORI.permisos & 2) != 2) { //Ambos necesitan leer y escribir para lo que viene ahora. Si falta uno de ellos, dejará su contenido vacio.
                if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("directorios.c/mi_copy") "La ruta '" TVAR(COLOR_CHILLON, "%s") "' no tiene permisos de lectura o escritura, imposible de replicar su contenido.\n",rutaOrigen);
                return 0; //[NO ES UN ERROR, ES UN AVISO] Dejarlo tal como esta, los pasos posteriores requieren de permisos adicionales que no los tiene presentes.
            }
            if(inodoORI.tipo == 'd') { //[RECURSVIO] Si es un directorio, anexo para cada entrada contenida su recursividad...
                nentradas = inodoORI.tamEnBytesLog / sizeof(struct entrada); //Contar la cantidad de entradas del inodo OBJETIVO.
                for(int i=0; i < nentradas; i++) {
                    mi_read_f(p_inodoORI, &proximaEntrada, i * sizeof(struct entrada), sizeof(struct entrada)); //Extraer la entrada del directorio.
                    strcpy(nuevoOrigen, rutaOrigen); //Clonar la ruta original.
                    strcat(nuevoOrigen, proximaEntrada.nombre); //Y transferir el nombre del objeto.
                    strcpy(nuevoDestino, generarRuta); //Clonar la ruta destino.
                    strcat(nuevoDestino, proximaEntrada.nombre); //Y transferir el nombre del objeto.
                    mi_copy(nuevoOrigen,nuevoDestino); //Recursividad sobre este...
                }
            } else if(inodoORI.tipo == 'f') { //Para el otro caso, duplicar los datos de la misma manera que fue usado por mi_cat.
                if(inodoORI.tamEnBytesLog > 0) { //Si hubiera datos, procedo a clonar. Si esta vacio, omitir el paso y dejarlo vacio.
                    //printf("'%s' sería escrito X datos desde el '%s'\n",generarRuta,rutaOrigen); //[DEBUG] Caso para fichero que tiene algo escrito.
                    bytesFichero = mi_read_f(p_inodoORI, bufferFichero, offsetFichero, BLOCKSIZE); //Primera lectura para entrar en el while...
                    while(bytesFichero > 0) { //Si ha procesado algo aunque haya huecos vacios...
                        if(memcmp(vacio, bufferFichero, BLOCKSIZE) != 0) { //[BUGFIX] strlen no logra distinguir los huecos vacios iniciales y se reemplaza por otra que si lo detecta. Si encuentra que tiene algo escrito...
                            nentradas = 0; //Reiniciar el offset y empezar a buscar a partir de donde tiene caracteres o datos...
                            for(int i=0; i < BLOCKSIZE; i++) {
                                if (bufferFichero[i] != 0) break; //Si encuentra algo, irrumpe y manten el valor de referencia 'nentradas'.
                                nentradas++;
                            }
                            strcpy(bufferFichero, bufferFichero+nentradas); //Desplazar y reubicar los objetos no vacios al principio para que el strlen de mi_write_f acepte el buffer.
                            mi_write_f(p_inodoDST, bufferFichero, offsetFichero+nentradas, bytesFichero-nentradas); //Respetar el offset de nulos y escribir desde ese punto, de los bytes leidos he de substraer (al desplazar el texto al principio). Funciona estando en BLOCKSIZE o marco inferior.
                        }
                        offsetFichero += bytesFichero; //Offset de la proxima iteración (tenga algo o no).
                        memset(bufferFichero, 0, BLOCKSIZE); //Prelimpieza para la siguiente vuelta del while (escrito o no).
                        if(offsetFichero < inodoORI.tamEnBytesLog) bytesFichero = mi_read_f(p_inodoORI, bufferFichero, offsetFichero, BLOCKSIZE); //Si aún puede seguir leyendo, procesa la siguiente entrada posible.
                        else bytesFichero = 0; //Si ya no puede seguir, marca el paso para finalizar.
                    }
                }
                //else printf("'%s' sería escrito 0 datos desde el '%s'\n",generarRuta,rutaOrigen); //[DEBUG] Caso para fichero vacio.
            }
            return 1; //Si estaba dentro, era aceptador o se ejecuto parcialmente los pasos (válidos).
        }
    }
    return -1;
}
