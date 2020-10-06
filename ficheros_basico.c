#include "ficheros_basico.h"

//Referido en punteros de los inodos y la cantidad de bloques de datos que puede albergar en cada uno.
#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) //256 (1024/4, tenga en cuenta que 1024 podría ser otro valor y tener más punteros o menos)
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS)                            //268 (256+12)
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0)             //65804 (65536+256+12)
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) //16843020 (16777216+65526+256+12)


//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                    EXTRAS
//
//Intermediario que delega montar la unidad disco y comprueba si este es válido. Aconsejable hacerlo inicialmente en los ejecutables para prevenir escrituras accidentales en ficheros que no sean de la unidad.
int bmountSAFE(const char *camino) {
    #ifdef REVISAR_UNIDAD
        int operativoBmount = bmount(camino); //Revisa si se abre o no la unidad de bloques. Le paso el valor de retorno posteriormente.
        if(operativoBmount >= 0) { //Apertura correcta.
            if(integridadDisco(1) > 0) { //Si la funcion extra de integridad da 1, acepta la combinación y es válido.
                return operativoBmount;
            } else { //Si experimenta algún error (inconsistente), abortar
                bumount();
            }
        }
        exit(EXIT_FAILURE); //Irrumpir la ejecución en su totalidad puesto que las otras instrucciones (de otras funciones) fallarian.
    #endif
    return bmount(camino); //Si se deshabilita del z_extra, sólo delegará su trabajo al bmount y no lo revisará.
}

//Comprueba a nivel de bytes y por superbloque, que sea válido la unidad de bloques antes de realizar cualquier manipulación.
//Retorno y su significado. { [-4,valores imposibles], [-3,problema en las fronteras por tipos], [-2,difiere el total del tamaño], [-1,problema de apertura], [0,error en bloques.c], [1,verificado y aceptado] }.
//"informarError" (boolean) 1 -- TRUE; 0 -- FALSE.  Define si ha de mostrar los errores o sólo mantener el return y desde afuera muestre el error general.
int integridadDisco(unsigned int informarError) {
    struct superbloque SB; //Estructura del superbloque
    off_t longitudBytes; //Para valores altamente grandes, inicialmente guarda los bytes del fichero y luego se rebaja a tener amplitud de bloques.
    longitudBytes = tamUnidad(); //Obtener la amplitud del fichero descriptor.
    if(longitudBytes >= 0) { //Fichero abierto.
        if((longitudBytes % BLOCKSIZE) != 0) {
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "El fichero no es una unidad de bloques, no es multiple de '%d'.\n",BLOCKSIZE);
            return -1;
        }
        longitudBytes = longitudBytes / BLOCKSIZE; //Reclamar bloques enteros.
        if(longitudBytes < minbloquesmkfs) {
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "La unidad de bloques es demasiado pequeña, ha de tener '%ld' (actualmente %d).\n",longitudBytes,minbloquesmkfs);
            return -1;
        }
        //Posterior a este punto, lee el superbloque y contrasta los valores totales para verificar su integridad. Los errores de retorno podrían ser usados para reconocer el error causado.
        bread(posSB, &SB); //Cargar los valores del superbloque.
        if(SB.totBloques != longitudBytes) { //Analiza si la totalidad coincide con la esperada.
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "Los bloques que maneja la unidad son incompatibles, se esperaba '" TVAR(COLOR_AMARILLO, "%ld") "' bloques pero hay '" TVAR(COLOR_CHILLON, "%d") "' (¿blocksize diferente?).\n",longitudBytes,SB.totBloques);
            return -2;
        }
        if(SB.posUltimoBloqueMB-SB.posPrimerBloqueMB != tamMB(longitudBytes)-1) {
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "Los campos del superbloque no son consistentes, el mapa de bits ha de ser '%d' (actualmente %d).\n",tamMB(longitudBytes)-1,SB.posUltimoBloqueMB-SB.posPrimerBloqueMB);
            return -3;
        }
        if(SB.posUltimoBloqueAI-SB.posPrimerBloqueAI != tamAI(longitudBytes/4)-1) {
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "Los campos del superbloque no son consistentes, el array de inodos ha de ser '%d' (actualmente %d).\n",tamAI(longitudBytes)-1,SB.posUltimoBloqueAI-SB.posPrimerBloqueAI);
            return -3;
        }
        if(SB.cantBloquesLibres > SB.totBloques || SB.cantInodosLibres > SB.totInodos) {
            if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "Los campos del superbloque tienen errores, los bloques libres (o de inodos) sobrepasan del limite establecido.\n");
            return -4;
        }
        if(RETURN_DEV) fprintf(stderr, MSG_OK("ficheros_basico.c/integridadDisco") "Unidad de bloques verificada y estable, bmount aceptado.\n");
        return 1; //Al no observar más problemas en esta capa, esta función acepta como válido la unidad.
    } else {
        if(informarError) fprintf(stderr, MSG_ERROR("ficheros_basico.c/integridadDisco") "Surgio un error al tratar de obtener la longitud total del fichero.\n");
    }
    return 0;
}


//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                    ETAPA 2
//
//Calcular el número de bloques necesarios del mapa de bits.
int tamMB(unsigned int nbloques)
{
    int primario = nbloques / 8; //Bytes a bit (paso 1)
    int residuo = primario % BLOCKSIZE; //Operador "(nbloques/8) % blocksize", determina si hay decimales.
    primario = primario / BLOCKSIZE;    //Tamaño del mapa de bits (paso 2) "(nbloques/8)/blocksize". Número de bloques minimos.
    if (residuo != 0)
    { //Tiene bits sueltos (hay decimales), necesita completar el bloque asignando 1 bloque más.
        primario++;
    }
    return primario; //Retorna el tamaño del mapa de bits.
}

//Calcular el número de bloques necesarios para los inodos.
int tamAI(unsigned int ninodos)
{
    int primario = ninodos * INODOSIZE; //Cantidad de bytes necesarios para todos los inodos (paso 1) (ninodos*128)
    int residuo = primario % BLOCKSIZE; //Operador "(ninodos*inodosize) % blocksize", determina si hay decimales.
    primario = primario / BLOCKSIZE;    //Tamaño del array de inodos (paso 2) "(ninodos*inodosize)/blocksize".
    if (residuo != 0)
    { //Tiene bits sueltos (hay decimales), necesita completar el bloque asignando 1 bloque más. (IDEM tamMB)
        primario++;
    }
    return primario; //Retorna el tamaño del array inodos.
}

// Crear el superbloque, mapa de bits, array inodos, reserva de bloques y del inodo raíz (facilita y resume el mi_mkfs).
int initSTRUCT(unsigned int nbloques)
{ //Funcion simplificada para mi_mkfs, da formato de estructura al SF.
    if (MODO_DEV)
        fprintf(stderr, MSG_INFO("ficheros_basico.c/initSTRUCT") "Inicializando la estructura del SF...\n");
    int ninodos = nbloques / 4; //Valor arbitrario y predeterminado.
    int retu; //Comprueba si tuvo error el formato.
    // initSB
    //if(MODO_DEV) printf(MSG_NOTA("ficheros_basico.c/initSTRUCT>>initSB") "Etapa 1 de 3\n");
    retu = initSB(nbloques, ninodos); //
    if (retu < 0)
    {
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/initSTRUCT<!initSB") "El superbloque tiene problemas, depuración necesaria.\n");
        exit(EXIT_FAILURE); //Detener el programa, habría que reparar para que funcione la base.
    }
    // initMB
    //if(MODO_DEV) printf(MSG_NOTA("ficheros_basico.c/initSTRUCT>>initMB") "Etapa 2 de 3\n");
    retu = initMB();
    if (retu < 0)
    {
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/initSTRUCT<!initMB") "El mapa de bits tiene problemas, depuración necesaria.\n");
        exit(EXIT_FAILURE); //Detener el programa, habría que reparar para que funcione la base.
    }
    // initAI
    //if(MODO_DEV) printf(MSG_NOTA("ficheros_basico.c/initSTRUCT>>initAI") "Etapa 3 de 3\n");
    retu = initAI();
    if (retu < 0)
    {
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/initSTRUCT<!initAI") "El array de inodos tiene problemas, depuración necesaria.\n");
        exit(EXIT_FAILURE); //Detener el programa, habría que reparar para que funcione la base.
    }
    //Reserva de bloques
    struct superbloque SB;
    bread(posSB, &SB); //Cargar los valores del superbloque.
    escribir_bit(posSB, 1); //Marca el bloque que aloja el superbloque como ocupado.
    SB.cantBloquesLibres--; //Detraer uno de los que tenia vacantes.
    for (int i = SB.posPrimerBloqueMB; i < SB.posPrimerBloqueDatos; i++)
    { //Desde el primero MB hasta el ultimo AI (supuestamente son CONTIGUAS)
        escribir_bit(i, 1); //Marcan los bloques como ocupadas.
        SB.cantBloquesLibres--; //Detraer los bloques disponibles.
    }
    bwrite(posSB, &SB); //Actualizar el valor de los bloques ocupados.
    //Reserva del inodo raíz
    struct inodo enraizar;
    leer_inodo(SB.posInodoRaiz, &enraizar); //Cargar los datos del inodo raíz (se diseño en un principio para el 0, no cambie el valor)
    if(enraizar.tipo == 'l') { //Comprueba que lo tiene disponible para reservar.
        int ignorar = reservar_inodo('d', (unsigned char)7); //Reserva el inodo, si difiere del 0, el programa daría un error.
        if(ignorar != SB.posInodoRaiz) exit(EXIT_FAILURE); //La reserva no tuvo efecto en el inodo raíz (No iba al 0, ¿a donde se fue la reserva?).
        if(MODO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/initSTRUCT") "Designado el inodo %d como directorio RAÍZ\n", ignorar); //Estado aceptador
    }
    //Finalización
    if (RETURN_DEV)
        fprintf(stderr, MSG_OK("ficheros_basico.c/initSTRUCT") "Estructura del SF desplegada (orden del mi_mkfs).\n"); //Mostrar confirmación.
    return 0; //Siempre cierto, a menos que un error irrumpa la construcción.
}

//Formato del superbloque al bloque especificado de metadatos.
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    SB.posPrimerBloqueMB = posSB + tamSB; //Bloque donde aloja el mapa de bits.
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1; //Frontera del mapa de bits.
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1; //Bloque donde aloja el array de inodos.
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1; //Frontera del array de inodos.
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1; //Bloque donde se guarda los datos y bloques de punteros.
    SB.posUltimoBloqueDatos = nbloques - 1; //Frontera de los datos posibles.
    SB.posInodoRaiz = 0; //Número del inodo raíz. El programa no fue diseñado para otro valor.
    SB.posPrimerInodoLibre = 0; //Inodo disponible para reservar, reservar_inodo ocupara ese inodo que tenga designado.
    SB.cantBloquesLibres = nbloques; //Cantidad de bloques que tiene disponibles la unidad, ha de ser la misma que fue especificada inicialmente.
    SB.cantInodosLibres = ninodos; //Cantidad de inodos disponibles, por lo general es 1/4 de los bloques (valor arbitrario).
    SB.totBloques = nbloques; //Cantitad de bloques que tiene la unidad, esta es fija y no debería de cambiar
    SB.totInodos = ninodos; // 1/4 de la cantidad de los bloques (valor arbitrario) 
    int formasb = bwrite(posSB, &SB); //Escribe los valores del superbloque.
    if (formasb < 0)
    { //Verifica si la escritura se ejecuto, si tuvo un error, saltará el mensaje de error.
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/initSB") "Hubo un problema al escribir el superbloque en el bloque '%d'\n", posSB);
        return -1;
    }
    return 0; //Superbloque construido correctamente.
}

//Formato del mapa de bits en los bloques de metadatos especificos.
int initMB()
{
    struct superbloque SB;
    bread(posSB, &SB); //Cargar los valores del superbloque.
    unsigned char fullcero[BLOCKSIZE]; //Bloque.
    memset(fullcero, 0, BLOCKSIZE); //Rellenar el bloque de 0's.
    for (int i = 0; i < tamMB(SB.totBloques); i++)
    { //Formato de mapa de bits en cada bloque requerido.
        bwrite(SB.posPrimerBloqueMB + i, fullcero);
    }
    return 0; //Mapa de bits construido correctamente. Siempre cierto.
}

//Formato de array de inodos en los bloques de metadatos especificos.
int initAI()
{
    struct inodo inodos[BLOCKSIZE / sizeof(struct inodo)];
    struct superbloque SB;
    bread(posSB, &SB); //Cargar los valores del superbloque.
    int siguienteInodo = SB.posPrimerInodoLibre + 1; //Para ubicar la centinela al ultimo inodo.
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    { //Formato de array de inodos en cada bloque requerido
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE) && siguienteInodo <= SB.totInodos; j++)
        { //Dentro de su bloque, a los inodos que caben dentro de este y no use más inodos de los indicados (bugfix)
            inodos[j].tipo = 'l'; //Asigna el inodo como estado "libre de asignar"
            if (siguienteInodo < SB.totInodos)
            { //El futuro inodo va a otro inodo, enlazar.
                inodos[j].punterosDirectos[0] = siguienteInodo;
            }
            else
            { //El futuro inodo sobresale del limite, designar la centinela al inodo (es el último)
                inodos[j].punterosDirectos[0] = UINT_MAX;
            }
            //if(MODO_DEV && (siguienteInodo <= 28 || siguienteInodo > SB.totInodos-21)) printf(MSG_VAR("inodo %d [mod %d]") "next pointer: %d\n", siguienteInodo-1, j, inodos[j].punterosDirectos[0]); //Usado para ver que hacia por dentro.
            siguienteInodo++; //Comprueba si el siguiente inodo sigue dentro de la frontera.
        }
        bwrite(i, inodos); //Escribe el grupo de inodos de su bloque.
    }
    return 0; //Array de inodos construido correctamente. Siempre cierto.
}

//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                    ETAPA 3
//
//Escribe en el mapa de bits la ocupación o liberación de un determinado bloque.
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    int posbyte = (nbloque / 8) % BLOCKSIZE; // Desplazamiento entre diferentes grupos de bytes enteros
    int posbit = nbloque % 8;                // 00 >> FF (byte entero, 2 nibbles)
    unsigned char colector[BLOCKSIZE];  //Bloque (una copia del bloque a través del bread)
    unsigned char mascara = 128; //Explicacion para el byte "(128,64,32,16 || 8,4,2,1)".
    mascara >>= posbit;          // ' 128/2^posbit '  Nibble izquierda "0"0 (128,64,32,16) o derecha 0"0" (8,4,2,1). (desplazamiento binario a la derecha)
    bread(posSB, &SB); //Cargar los valores del superbloque.
    bread(SB.posPrimerBloqueMB + ((nbloque / 8) / BLOCKSIZE), colector); //Coger el bloque que alberga el grupo de bits, entre ellos el que refiere al nbloque.
    if (bit == 0)
    {                                  //Asignar en el byte mod, el bit 0.
        colector[posbyte] &= ~mascara; //El operador AND, asigna el bit 0 sobre el byte ubicado.
    }
    else if (bit == 1)
    {                                 //Asignar en el byte mod, el bit 1.
        colector[posbyte] |= mascara; //El operador OR, asigna el bit 1 sobre el byte ubicado.
    }
    else
    { //El argumento BIT es desconocido, mal uso de la función.
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros_basico.c/escribir_bit") "El argumento BIT '%d' no es 1 (ocupado) ni 0 (libre), omision.\n", nbloque);
        return -1;
    }
    bwrite(SB.posPrimerBloqueMB + ((nbloque / 8) / BLOCKSIZE), colector); //Actualizar el grupo de bits junto el nuevo bit del bloque.
    return 0; //Asignación del bit de estado actualizado.
}

//Mira si el bloque pedido esta en uso o disponible.
unsigned char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;
    int posbyte = (nbloque / 8) % BLOCKSIZE;  // Desplazamiento entre diferentes grupos de bytes enteros
    int posbit = nbloque % 8;                 // 00 >> FF (byte entero, 2 nibbles)
    int bloqueMB = (nbloque / 8) / BLOCKSIZE; //Determina el bloque relativo (desde el principio del mapa de bits) donde se va a ubicar.
    unsigned char colector[BLOCKSIZE];
    unsigned char mascara = 128; //Explicacion para el byte "(128,64,32,16 || 8,4,2,1)".
    //
    bread(posSB, &SB); //Cargar los valores del superbloque.
    bread(SB.posPrimerBloqueMB + bloqueMB, colector); //Coger el bloque que alberga el grupo de bits, entre ellos el que refiere al nbloque.
    mascara >>= posbit;           // ' 128/2^posbit '  Nibble izquierda "0"0 (128,64,32,16) o derecha 0"0" (8,4,2,1). (desplazamiento binario a la derecha)
    mascara &= colector[posbyte]; //Prueba con un AND si el 128/2^posbit es valido.
    mascara >>= (7 - posbit);     //Si devuelve su valor '128/2^posbit' (AND aceptado), hará el desplazamiento de bits (binario) hacia a la derecha
    return mascara; //El valor del bit; '1' ocupado, '0' libre.
}

//Ocupar un bloque de datos. (me falta ampliar el comentario)
int reservar_bloque()
{
    struct superbloque SB;
    unsigned char mascara = 128;        //Lo volvemos a necesitar para el cálculo de los bits.
    unsigned char colector[BLOCKSIZE];  //Recorre cada bloque el mapa de bits.
    unsigned char bufferAux[BLOCKSIZE]; //El formato de un bloque 'totalmente' ocupado esta compuesto todo en "ff" (todo en valor 1).
    memset(bufferAux, 255, BLOCKSIZE);  //Formato de exclusion (previamente indicado).
    int auxiliar;                       //Averigua el valor del return.
    int posBloqueMB;                    //Guarda el número del bloque del mapa de bits donde fue leido (el no excluido).
    int posbyte = -1;                   //Busca el offset entre los diferente bytes hasta que alguno no tenga "ff".
    int posbit = -1;                    //Desde el posbyte, evalua el bit (los 8 que componen el byte).
    int nbloque = -1;                   //El bloque resultante.
    //
    bread(posSB, &SB); //Cargar los valores del superbloque.
    if (SB.cantBloquesLibres <= 0)
    { //Cuando se pide un imposible...
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/reservar_bloque") "Capacidad maxima de bloques, no queda ninguno '%d'.\n", SB.cantBloquesLibres);
        return -1;
    }
    for (posBloqueMB = SB.posPrimerBloqueMB; posBloqueMB <= SB.posUltimoBloqueDatos; posBloqueMB++)
    {                                                      //Recorrer en el mapa de bits, en busqueda de un bloque que no tenga todo "ff" (255, todos ocupados)
        bread(posBloqueMB, colector);                      //Extrae el bloque para consultar el contenido.
        auxiliar = memcmp(colector, bufferAux, BLOCKSIZE); //Compara el bloque colector con el de bloqueexclusion, si son iguales, he de seguir buscando en otro bloque de bits.
        if (auxiliar != 0)
            break;
    }
    for (int i = 0; i < BLOCKSIZE; i++)
    { //Tengo el bloque que como minimo 1 o más esta disponible (alguno de ellos es != 255), busco en cada posible byte dentro del rango.
        if (colector[i] != 255)
        { //Lo encontre, aqui esta uno de los grupos que buscaba (!= 255).
            posbyte = i;
            break;
        }
    }
    auxiliar = colector[posbyte]; //El caso de la profe lo hace con el nombre 'byte' (3.1, pag 3).
    posbit = 0; //Desplazo en busqueda del primer bit.
    while (auxiliar & mascara)
    {                   //Busca dentro del BYTE, el bit que esta a 0.
        auxiliar <<= 1; //Desplazar a la izquierda (binario)
        posbit++;
    }
    nbloque = ((posBloqueMB - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit; //Ahora que ya conozco toda la ubicación (bloque, grupo del byte y su bit pertinente), determino que bloque esta haciendo referencia.
    if (nbloque < 0)
        return -1; //Seria raro que surja este error, pero sigue existiendo esa posibilidad. Normalmente pasaría por error de cálculo o por medio de un sabotaje estructural.
    auxiliar = escribir_bit(nbloque, 1); //Intentar reservar el bloque referido.
    if (auxiliar >= 0)
    { //El bloque pedido me ha sido asignado, confirmo la reserva.
        SB.cantBloquesLibres--; //Detraer el bloque del recuento disponible.
        bwrite(posSB, &SB); //Actualiza con el nuevo bloque referido.
        //Ahora hay que limpiar el contenido de este (por si fuera a tener el papel de bloque de punteros).
        memset(bufferAux, 0, BLOCKSIZE); //PreLimpiar el bloque (formatear).
        bwrite(nbloque, bufferAux); //Solapar todo dato residual por este bloque vacio.
        if (BIT_DEV)
            fprintf(stderr, MSG_INFO("ficheros_basico.c/reservar_bloque") "Bloque reservado '%d' ⇒ posbyte:bit %d:%%%d, Mapa: %d. Quedan %d bloque(s).\n", nbloque, posbyte, posbit, posBloqueMB, SB.cantBloquesLibres); //Revision minuciosa del circuito interno y de los calculos si me lo piden.
        return nbloque; //El bloque fisico reservado.
    }
    return -1; //Error interno de calculos.
}

//Retorna el bloque usado a estar nuevamente disponible para su asignación.
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    bread(posSB, &SB); //Cargar los valores del superbloque.
    int reto = leer_bit(nbloque); //Preguntar al mapa de bits el estado del bloque.
    if (reto > 0)
    { //Esta comprobado que el bloque esta en uso, se procede a liberar.
        escribir_bit(nbloque, 0); //Designar el bloque como libre.
        SB.cantBloquesLibres++; //Aumentar la cantidad de bloques disponibles.
        if (BIT_DEV)
            fprintf(stderr, MSG_INFO("ficheros_basico.c/liberar_bloque") "Bloque liberado '%d' ⇒ bloques disponibles ahora: %d.\n", nbloque, SB.cantBloquesLibres);
        bwrite(posSB, &SB); //Actualizar el valor de bloques disponibles.
        return 0; //Bloque liberado.
    }
    //El bloque especificado sigue libre, omitir con un aviso menor.
    if (reto == 0 && WARNINGS_LIB)
        fprintf(stderr, MSG_AVISO("ficheros_basico.c/liberar_bloque") "El bloque '%d' no estaba ocupado, omision.\n", nbloque); //Posiblemente fuera algun despiste.
    return -1; //Error porque no estaba ocupado previamente.
}

//Reemplaza el contenido del inodo por otro actualizado.
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct superbloque SB;
    struct inodo bufferInodos[BLOCKSIZE / INODOSIZE]; //Grupo de inodos de 1 bloque.

    bread(posSB, &SB); //Cargar los valores del superbloque.
    bread(SB.posPrimerBloqueAI + (ninodo * INODOSIZE / BLOCKSIZE), bufferInodos); //Leer el bloque que contiene del grupo de inodos, el inodo especificado.
    bufferInodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo; //Reemplazar sólo el inodo pedido, los otros permanecen invariables.
    bwrite(SB.posPrimerBloqueAI + (ninodo * INODOSIZE / BLOCKSIZE), bufferInodos); //Actualiza el bloque del grupo de inodos
    if(INODO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/escribir_inodo") "Inodo %d (bloque '%d') escrito en la posicion %d del buffer.\n", ninodo, SB.posPrimerBloqueAI + (ninodo * INODOSIZE / BLOCKSIZE), ninodo % (BLOCKSIZE / INODOSIZE)); //Compactar la información y simplificación.
    bwrite(posSB, &SB); //Actualiza el valor del superbloque. (¿se había cambiado algo? no veo indicios)

    return 0; //Siempre cierto.
}

//Carga el contenido del inodo y lo envia.
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    struct inodo bufferInodos[BLOCKSIZE / INODOSIZE]; //Grupo de inodos de 1 bloque.

    bread(posSB, &SB); //Cargar los valores del superbloque.
    bread(SB.posPrimerBloqueAI + (ninodo * INODOSIZE / BLOCKSIZE), &bufferInodos); //Leer el bloque que contiene del grupo de inodos, el inodo especificado.

    *inodo = bufferInodos[ninodo % (BLOCKSIZE / INODOSIZE)]; //Traslada el inodo pedido y lo envia.

    if(INODO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/leer_inodo") "Inodo %d (bloque '%d') leido en la posicion %d del buffer, procesado.\n", ninodo, SB.posPrimerBloqueAI + (ninodo * INODOSIZE / BLOCKSIZE),ninodo % (BLOCKSIZE / INODOSIZE)); //Indicar como fue leido el inodo, similar al de escribir.
    return 0; //Siempre cierto.
}

//Ocupar un inodo disponible (arreglado el problema de recuperar el siguiente inodo libre).
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodoEscribir;
    struct inodo leerPuntero; //Posible arreglo para el inodo libre a ocupar.
    unsigned int posInodoReservado = 0; //Guarda el número del inodo que fue reservado.

    bread(posSB, &SB); //Cargar los valores del superbloque.
    if (SB.cantInodosLibres <= 0)
    { //Ya no queda ninguno disponible, la reserva fallaría.
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/reservar_inodo") "No hay inodos suficientes para reservar.\n");
        return -1;
    }
    posInodoReservado = SB.posPrimerInodoLibre; //Obtener el número del inodo reservado.
    leer_inodo(posInodoReservado, &leerPuntero); //Hay que leer el inodo para mangarle el puntero[0] que refiere el siguiente inodo.
    SB.posPrimerInodoLibre = leerPuntero.punterosDirectos[0]; //Extrae el número del inodo de su lista enlazada.
    //printf("El puntero del inodo '%d' es el directo0 '%d'\n",posInodoReservado,SB.posPrimerInodoLibre); //Usado para revisar.

    //Redefine los valores del inodo a su estado inicial.
    inodoEscribir.tipo = tipo;
    inodoEscribir.permisos = permisos;
    inodoEscribir.nlinks = 1;
    inodoEscribir.tamEnBytesLog = 0;
    inodoEscribir.ctime = time(NULL);
    inodoEscribir.atime = time(NULL);
    inodoEscribir.mtime = time(NULL);
    inodoEscribir.numBloquesOcupados = 0;
    memset(inodoEscribir.punterosDirectos, (unsigned int)0, 12 * sizeof(unsigned int));
    memset(inodoEscribir.punterosIndirectos, (unsigned int)0, 3 * sizeof(unsigned int));

    escribir_inodo(posInodoReservado, inodoEscribir); //Actualiza el inodo y lo guarda.

    SB.cantInodosLibres--; //Detraer el inodo reservado.
    bwrite(posSB, &SB); //Actualizar los valores del superbloque.

    if(INODO_DEV) fprintf(stderr, MSG_NOTA("ficheros_basico.c/reservar_inodo") "Se ha reservado el inodo '%d', quedan %d inodo(s).\n", posInodoReservado, SB.cantInodosLibres); //Mostrar si me lo piden, que inodo me ha reservado y el contador restante.
    return posInodoReservado; //El número del inodo reservado.
}


//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                    ETAPA 4
//
//Determina el grado del puntero del bloque logico.
int obtener_nRangoBL(struct inodo inodo, unsigned int nblogico, unsigned int *ptr)
{
    //Obtenemos el nivel al que pertenece y se devuelve
    if (nblogico < DIRECTOS)
    { //Los 12 primeros.
        *ptr = inodo.punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    { //El indirecto0 + un offset de 12.
        *ptr = inodo.punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    { //El indirecto1 + un offset de 268
        *ptr = inodo.punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    { //El indirecto2 + un offset de 65804
        *ptr = inodo.punterosIndirectos[2];
        return 3;
    }
    else
    { //En caso de error (fuera de lugar porque es superior al limite superior), *ptr = 0 y perror
        *ptr = 0;
        fprintf(stderr, MSG_ERROR("ficheros_basico.c/obtener_nRangoBL") "Bloque logico '%d' fuera del rango valido\n",nblogico); //Faltaba el mensaje de error.
        perror("Bloque lógico fuera de rango");
        return -1;
    }
}

//Calculo del indice según el grado del puntero logico.
int obtener_indice(int nblogico, int nivel_punteros)
{
    //Según los valores de entrada se calcula la posición del índice
    if (nblogico < DIRECTOS)
    { //Si son exclusivamente los 12 primeros.
        if(nivel_punteros == 0) return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    { //Asociado al indirecto0.
        if(nivel_punteros == 1) return (nblogico - DIRECTOS);
    }
    else if (nblogico < INDIRECTOS1)
    { //El indirecto1 intermedio, suele ser el central.
        if (nivel_punteros == 2)
        { //Asociado al indirecto1.
            return ((nblogico - INDIRECTOS0) / NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        { //Bloque lejano.
            return ((nblogico - INDIRECTOS0) % NPUNTEROS);
        }
    }
    else if (nblogico < INDIRECTOS2)
    { //El indirecto2 alargado, suele ser el que cuelga todo.
        if (nivel_punteros == 3)
        { //Asociado al indirecto2.
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));
        }
        else if (nivel_punteros == 2)
        { //Intermedio.
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        { //Bloque lejano.
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }
    }
    return -1;
}

//Asigna los bloques de punteros al inodo para guardar los datos, al final del proceso le devuelve el bloque fisico reservado.
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, char reservar)
{
    struct inodo inodo;
    unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice = 0;
    unsigned int buffer[NPUNTEROS]; //Buffer de un bloque de datos exclusivamente en punteros (BLOCKSIZE/4)
    leer_inodo(ninodo, &inodo); //Carga el inodo indicado.
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); //Determina el grado de profundidad del puntero.
    nivel_punteros = nRangoBL; //Grado de profundidad del puntero, duplicado para manejar.
    while (nivel_punteros > 0)
    { //Mientras este recorriendo una arista de punteros (no tendrá efecto si es un puntero directo)...
        if (ptr == 0)
        { //Si esta vacio...
            if (reservar == 0)
            {
                return -1; //error de lectura bloque
            }
            else
            { //Procede a reservar el bloque de punteros para el grado del puntero.
                salvar_inodo = 1;
                ptr = reservar_bloque(); //Obtener el bloque de datos para designarlo como punteros.
                inodo.numBloquesOcupados++; //Agrega el bloque de punteros, sigue siendo uno que usa el inodo.
                inodo.ctime = time(NULL); //Designa el tiempo de creación al inodo.
                if (nivel_punteros == nRangoBL)
                { //Si es el bloque de profundidad más alta, genera una referencia al indirectoX un bloque de punteros.
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr; //Escribo el indirectoX la ruta asociada del bloque de punteros.
                    if(PUNTERO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/traducir_bloques_inodo")"inodo.punterosIndirectos[%d] = %d (Reservado BF %d para punteros_nivel%d).\n", nRangoBL-1, ptr, ptr, nivel_punteros);
                }
                else
                {
                    buffer[indice] = ptr; //Escribo al bloque de punteros.
                    if(PUNTERO_DEV) fprintf(stderr, MSG_VAR("ficheros_basico.c/traducir_bloques_inodo")"punteros_nivel%d [%d] = %d (Reservado BF %d para punteros_nivel%d).\n", nivel_punteros+1, indice, ptr, ptr, nivel_punteros);
                    bwrite(ptr_ant, buffer); //Guarda el puntero al bloque de punteros.
                }
            }
        }
        bread(ptr, buffer); //Leer el bloque de punteros que apunta la arista.
        indice = obtener_indice(nblogico, nivel_punteros); //Determina el futuro indice para el siguiente bloque de punteros acorde al futuro grado.
        ptr_ant = ptr;        //guardamos puntero
        ptr = buffer[indice]; //y desplazamos nivel
        nivel_punteros--; //Bajamos el grado de punteros.
    } //al salir del bucle estamos a nivel de datos o de los directos.

    if (ptr == 0)
    { //Si esta vacio...
        if (reservar == 0)
        {
            return -1; //Error
        }
        else
        { // Procede a reservar un bloque.
            salvar_inodo = 1;
            ptr = reservar_bloque(); //Obtener el bloque de datos para designarlo como punteros.
            inodo.numBloquesOcupados++; //Agrega el bloque de punteros, sigue siendo uno que usa el inodo.
            inodo.ctime = time(NULL); //Designa el tiempo de creación al inodo.
            if (nRangoBL == 0)
            { //Si es el grado de un puntero directo.
                inodo.punterosDirectos[nblogico] = ptr;
                if(PUNTERO_DEV) fprintf(stderr, MSG_VAR("ficheros_basico.c/traducir_bloques_inodo")"inodo.punterosDirectos[%d] = %d (Reservado BF %d para BL %d).\n", nblogico, ptr, ptr, nblogico); //A ese print no le concierne el rangoBL (porque es 0)
            }
            else
            { //En caso contrario, lo guardo al bloque de punteros.
                buffer[indice] = ptr; //Escribo al bloque de punteros.
                if(PUNTERO_DEV) fprintf(stderr, MSG_VAR("ficheros_basico.c/traducir_bloques_inodo")"punteros_nivel%d [%d] = %d (Reservado BF %d para BL %d).\n", nivel_punteros+1, indice, ptr, ptr, nblogico); //No mostraba la información pertinente.
                bwrite(ptr_ant, buffer); //Guarda el puntero al bloque de punteros.
            }
        }
    }
    if (salvar_inodo == 1)
    { //Actualizar el inodo si hubo cambios (agrego un bloque).
        escribir_inodo(ninodo, inodo); //solo si se ha actualizado
    }
    return ptr; //El número del bloque fisico para su uso de datos.
}


//
//  ---     ---     ---     ---     ---     ---     ---     ---     ---     ---
//                                    ETAPA 5
//
//Retorna el inodo usado a estar nuevamente disponible para su asignación. Para ello, ha de liberar todos los bloques que tiene ocupados.
int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo;
    struct superbloque SB;
    unsigned int bloquesReservados = 0; //Cantidad de bloques usados.

    //Leemos todos los bloques del inodo empezando por el 0 hasta el final
    bloquesReservados = liberar_bloques_inodo(ninodo, 0); //Liberar todos los bloques asociados a este inodo. Esto le puede abarcar un cierto momento.
    leer_inodo(ninodo, &inodo); //Cargar los valores del inodo.
    inodo.tipo = 'l'; //Marcamos el tipo como uno libre.
    if(inodo.numBloquesOcupados != 0 && WARNINGS_LIB) { //Si al liberar, detecto que difieren los bloques que tenian que liberarse, dar error.
        fprintf(stderr, MSG_AVISO("ficheros_basico.c/liberar_inodo") "El inodo '%d' ha sobrevivido '%d'/%d bloque(s) y deberia de ser 0 bloques.\n", ninodo, inodo.numBloquesOcupados, bloquesReservados);
    }
    //Actualizar la lista enlazada de inodos libres
    bread(posSB, &SB); //Hay que leer el superbloque aquí, ya que la función liberar_bloques_inodo cambia el valor de SB.cantBloquesOcupados.
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre; //Anexa el inodo liberado la referencia del siguiente inodo que tiene disponible.
    SB.posPrimerInodoLibre = ninodo; //Una vez anexado la ruta, le asigna a este inodo libre como proximo inodo a reservar.
    if(INODO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/liberar_inodo") "Se ha liberado el inodo '%d' y sus '%d' bloque(s), puntero directo[0] apunta ahora a '%d'\n", ninodo, bloquesReservados, inodo.punterosDirectos[0]);
    SB.cantInodosLibres++; //Aumentamos la cantidad de inodos libres aquí, y la cantidad de bloques libres no hay que aumentarla ya que se aumenta en la función "liberar_bloque" en la llamada a "liberar_bloques_inodo"
    escribir_inodo(ninodo, inodo); //Ahora escribimos el SB y los campos actualizados del inodo liberado.
    bwrite(posSB, &SB);

    return ninodo; //Devuelve el inodo liberado.
}

//Liberar los bloques de (datos/punteros) que estan en uso por el inodo.
int liberar_bloques_inodoLEGACY(unsigned int ninodo, unsigned int nblogico)
{
    //Definición variables
    struct inodo inodo;
    unsigned int nRangoBL;
    unsigned int nivel_punteros;
    unsigned int indice;
    unsigned int ptr;
    unsigned int nblog;
    unsigned int ultimoBL; //El ultimo bloque logico posible.
    unsigned char buffer[BLOCKSIZE]; //Usado para el memcmp como un bloque vacio.
    int bloques_punteros[3][NPUNTEROS]; //array de bloques de punteros
    int ptr_nivel[3];                   //punteros a bloques de punteros de cada nivel
    int indices[3];                     //Indices de cada nivel
    int liberados;                      //nº de bloques liberados

    //Calculamos el último bloque lógico del fichero
    liberados = 0; //Cantidad de bloques que fueron liberados.
    memset(buffer, 0, BLOCKSIZE); //Vaciar el buffer.
    leer_inodo(ninodo, &inodo); //Leer los datos del inodo.
    // Para hacer la prueba de la etapa 5, se tiene que comentar "bytes logicos" porque no esta implementada aún.
    if (inodo.tamEnBytesLog == 0)
    {
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros_basico.c/liberar_bloques_inodo") "tamEnBytesLog == 0 (¿el inodo estaba vacio?), omision.\n");
        return 0; //El inodo está vacío, no tiene datos.
    }
    //Obtenemos el último bloque lógico, determinado por el número de bytes lógicos escritos que tiene en uso.
    if ((inodo.tamEnBytesLog % BLOCKSIZE) == 0)
    { //En esta circustancia, si son bloques exactos, excluimos 1.
        ultimoBL = ((inodo.tamEnBytesLog / BLOCKSIZE) - 1);
    }
    else
    { //Para esta otra es si tiene algún bloque parcialmente en uso.
        ultimoBL = (inodo.tamEnBytesLog / BLOCKSIZE);
    }
    //ultimoBL = 16843019; //Exigido en la prueba de la etapa5, definir que busque en todos los punteros del inodo (palo ciego).
    ptr = 0; //Inicializo el valor inicial, desde el for este ira cambiando.
    for (nblog = nblogico; nblog <= ultimoBL; nblog++)
    { //recorrido BLs
        if(MODO_DEV && nblog % 3000000 == 0) fprintf(stderr, MSG_NOTA("ficheros_basico.c/liberar_bloques_inodo<!FOR") "Intervalo procesado %d (%d restantes).\n", nblog, ultimoBL-nblog); //Ir mostrando como evoluciona los pasos intermedios (una buena forma de saber que no se ha calado).
        nRangoBL = obtener_nRangoBL(inodo, nblog, &ptr); //Determina el grado del puntero (si es uno indirecto)
        if (nRangoBL < 0)
        { //Tuvo un error al posicionar, posiblemente sobrepase la frontera del indirecto2.
            fprintf(stderr, MSG_ERROR("ficheros_basico.c/liberar_bloques_inodo") "nRangoBL '%d' es inferior a 0, crash.\n",nRangoBL); //Faltaba el mensaje de error.
            perror("Valor de nRangoBL negativo!");
            exit(EXIT_FAILURE); //Entraba en un bucle infinito y no llegaba a finalizar.
        }
        nivel_punteros = nRangoBL; //nivel_punteros +alto que cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { //Mientras tenga algun bloque referido y algún grado de puntero.
            bread(ptr, bloques_punteros[nivel_punteros - 1]); //Extraer el bloque referido ptr los datos del bloque punteros.
            indice = obtener_indice(nblog, nivel_punteros); //Buscar el indice el bloque logico contenido.
            ptr_nivel[nivel_punteros - 1] = ptr; //Asocio el bloque de punteros del grado.
            indices[nivel_punteros - 1] = indice; //Recordar el indice del grado actual.
            ptr = bloques_punteros[nivel_punteros - 1][indice]; //Conservo el bloque de punteros que he de liberar posteriormente.
            nivel_punteros--;
        }

        if (ptr > 0)
        { //Hay un bloque de punteros, vamos a liberar su contenido antes de desacoplar.
            liberar_bloque(ptr); //Retiro el bloque logico final.
            liberados++; //Agrego el bloque retirado.
            if (nRangoBL == 0)
            { //Si era uno directo, simplemente lo desenlaza.
                inodo.punterosDirectos[nblog] = 0;
            }
            else
            { //Es indirecto, habrá que desenlazar todo el contenido del bloque de punteros.
                while (nivel_punteros < nRangoBL)
                { //Hasta que no procese todos los grados que tenia en uso.
                    indice = indices[nivel_punteros]; //Recuperar el indice del grado actual (del paso previo)
                    bloques_punteros[nivel_punteros][indice] = 0; //Desenlaza el bloque de datos del bloque punteros.
                    ptr = ptr_nivel[nivel_punteros]; //Recuperar el bloque de punteros que he de examinar.
                    if (memcmp(bloques_punteros[nivel_punteros], buffer, BLOCKSIZE) == 0)
                    { //Si el bloque de punteros ya esta vacio...
                        liberar_bloque(ptr); //Libero el bloque de punteros.
                        liberados++; //Agrego la cuenta.
                        nivel_punteros++; //Avanzo al bloque de punteros de mayor grado (acercarse al inodo contenido).
                        if (nivel_punteros == nRangoBL)
                        { //Si este es el que refiere al puntero del indirecto base del inodo, la marco como libre.
                            inodo.punterosIndirectos[nRangoBL - 1] = 0;
                        }
                    }
                    else
                    { //Si aún queda datos en el bloque de punteros...
                        bwrite(ptr, bloques_punteros[nivel_punteros]); //Simplemente dejo el bloque de datos del bloque punteros como libre (del paso previo), el bloque de puntero aún ha de permanecer para las proximas iteraciones.
                        nivel_punteros = nRangoBL; //Linea para salir del bucle.
                    }
                }
            }
        }
    }
    if(RETURN_DEV) fprintf(stderr, MSG_OK("ficheros_basico.c/liberar_bloques_inodo") "Se anularon %d bloque(s) del inodo '%d'.\n", liberados, ninodo); //Mensaje de confirmación si lo necesitará.
    inodo.numBloquesOcupados -= liberados; //Detraer los bloques que fueron ocupados. Este valor debería de ser 0 al procesar.
    escribir_inodo(ninodo, inodo);
    return liberados; //El número de bloques que han sido anulados del inodo.
}

//Selector de la version RECURSIVA y la TRADICIONAL del liberar los bloques de (datos/punteros) que estan en uso por el inodo.
int liberar_bloques_inodo(unsigned int ninodo, unsigned int nblogico) {
    if(LIBERAR_RECURSIVO) {
        //return liberar_bloques_inodo_recursivo(ninodo,nblogico); //Version obsoleta original de la profe. Fue tan confuso la del profe que hubo necesidad de substituir por otra más simple.
        return liberar_bloques_inodoRECURSIVO(ninodo,nblogico); //Version recursiva; en términos de procesamiento, este presenta mejor rendimiento [ACELERADO].
    } else {
        return liberar_bloques_inodoLEGACY(ninodo,nblogico); //Version tradicional; usarla en caso de que exista trabas o no se liberen debidamente [COMPATIBILIDAD].
    }
}

//Liberar los bloques de (datos/punteros) que estan en uso por el inodo con ayuda de la recursividad.
int liberar_bloques_inodoRECURSIVO(unsigned int ninodo, unsigned int nblogico) {
    struct inodo inodo;
    unsigned int basurero = 0; //Previene el warning del "obtener_nRangoBL", la variable en si no se dará uso.
    int nblimite = 0; //Variable delimitadora para la "frontera superior". Permanece como constante entre las funciones.
    int nliberados = 0; //Número de "bloques que ha liberado" desde entonces. Preferiblemente ha de compartirse.
    int nprocesados = 12; //"Número de bloques procesados", este actua como una variable I de for hasta nliberados. Se ha COMPARTIR su valor entre las funciones. Siempre empieza 12 después del FOR directos.
    int rangoindirecto = 0; //Marco de "profundidad de los indirectos" a liberar (disparador recursivo).
    int retornadoindirecto = 0; //He de recuperar el estado de la recursividad, si me devuelve algo distinto de 0 es que ha quedado vacio y lo puedo vaciar.
    leer_inodo(ninodo, &inodo); //Necesitare leer ahora el inodo para conocer la totalidad de bloques que puede tener.

    //Reconocer el bloque limite (los bloques que esten por encima de este valor están vacios y se puede abortar prematuramente)
    if (inodo.tamEnBytesLog % BLOCKSIZE == 0) nblimite = inodo.tamEnBytesLog / BLOCKSIZE; //Si es un bloque entero...
    else nblimite = (inodo.tamEnBytesLog / BLOCKSIZE) + 1; //Si esta fraccionado el bloque, ponderación hacia arriba (siguiente bloque).
    
    if(nblimite > 0) rangoindirecto = obtener_nRangoBL(inodo,nblimite-1,&basurero); //Determinar como de grande va a ser los indirectos (hay que detraer 1 puesto que usa 12 < 12 como directo). El basurero no se da uso y lo pongo de relleno.

    for(int i=nblogico; i < 12 && i < nblimite; i++) { //Liberar los directos, siempre y cuando el nblogico no los excluya y no alcance el limite superior (inodo diminuto).
        if(inodo.punterosDirectos[i] > 0 && liberar_bloque(inodo.punterosDirectos[i]) >= 0) { //Si el inodo tiene el bloque de datos en uso y logro liberarlo...
            if(PUNTERO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/liberar_bloques_inodoRECURSIVO") "Directo[%d], bloque '%d' anulado del inodo.\n",i,inodo.punterosDirectos[i]);
            inodo.punterosDirectos[i] = 0; //Inhabilitar el bloque de datos del directo (liberado).
            nliberados++; //Agregar en la cuenta el número de bloques retirados.
        }
    }

    for(int i=0; i < rangoindirecto; i++) { //Para los indirectos, determinado por el "obtener_nRangoBL" mi limitación.
        retornadoindirecto = liberador_punteros_datosRECURSIVO(inodo.punterosIndirectos[i], nblogico, i, nblimite, &nliberados, &nprocesados); //Delego la tarea de liberar a la función de bloque punteros.
        if(retornadoindirecto > 0) { //Si me advierten que he de liberar, pedir permiso para liberar.
             if(liberar_bloque(inodo.punterosIndirectos[i]) >= 0) { //Si el mapa de bits da conformidad, anulo la ruta del bloque.
                if(PUNTERO_DEV) fprintf(stderr, MSG_INFO("ficheros_basico.c/liberar_bloques_inodoRECURSIVO") "Indirecto[%d]>>[*] -- RAIZ, bloque de punteros '%d' anulado del inodo.\n",i,inodo.punterosIndirectos[i]);
                inodo.punterosIndirectos[i] = 0; //Inhabilitar el bloque de punteros del indirecto (liberado).
                nliberados++; //Agregar en la cuenta el número de bloques retirados.
             }
        }
    }


    if(nliberados > 0) { //Si se ha liberado al menos 1 bloque de lo que sea, actualizar.
        inodo.numBloquesOcupados -= nliberados; //Detraer los bloque retirados de la capacidad.
        escribir_inodo(ninodo, inodo); //Si por donde fuera, se ha liberado algun bloque cualesquiera.
        if(RETURN_DEV) fprintf(stderr, MSG_OK("ficheros_basico.c/liberar_bloques_inodoRECURSIVO") "Se anularon %d bloque(s) del inodo '%d'.\n", nliberados, ninodo); //Mensaje de confirmación si lo necesitará.
    } else {
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros_basico.c/liberar_bloques_inodoRECURSIVO") "El marco de exclusión omitio todos los bloques, inodo '%d' sin cambios.\n",ninodo);
    }
    return nliberados; //Indistintamente de lo que pase, siempre le daré el número que he quitado.
}

//Tratamiento de los indirectos "puntero a puntero/datos", esta función esta asignada como RECURSIVO por parte de "liberar_bloques_inodoRECURSIVO". Con los nuevos arreglos ahora puede funcionar en todas las situaciones a gran velocidad; puede llegar a ser virtualmente instantaneo (1 ms).
int liberador_punteros_datosRECURSIVO(unsigned int bloquepuntero, unsigned int bloqueminimo, int nivel_punteros, int nblimite, int *nliberados, int *nprocesados) {
    //La funcion dependera del nivel_punteros...
    // == 0 "punteros a datos", liberar los bloques de la misma manera que hice en directos.
    // > 0 "punteros a punteros", generar recursividad con nivel_punteros-1; los bloques punteros de datos son quienes se encargan de liberarse por si mismos.
    
    // RETURN "== 0; el bloque a si mismo sigue operativo, no retirarlo", "> 0; el bloque de punteros puede liberarse, no queda nada dentro (me lo informaran los extremos finales)"

    //Desde el parametro he de requerir como valores para manejar y mantener simple la idea...
    //El bloque de datos que contiene el número del bloque que tendrá los punteros de referencia a liberar. Usar el "buffer[NPUNTEROS]" como ayuda.
    //Nivel_punteros me servira para determinar el comportamiento y en la forma en como se retira los bloques.
    //nblimite es mi variable CONSTANTE, actua como delimitador SUPERIOR, por encima de este no tendrá ningún bloque de datos.
    //nliberados es la COMPARTIDA, alli voy indicando los bloques que se han ido liberando.
    //nprocesados es la COMPARTIDA, se encarga de ir regulando la exclusion inicial como si fuera nblogico y es el que DETIENE el ciclo del for.
    unsigned int DatosOmitidos[3] = { NPUNTEROS, NPUNTEROS*NPUNTEROS, NPUNTEROS*NPUNTEROS*NPUNTEROS }; //[BUGFIX] Cantidad bloques de datos que tiene cada nivel de punteros, usado para agregar rapidamente a la recursiva el seguimiento.

    if(bloquepuntero == 0) { //Detectar si me llamaron sin tener posibilidad de actuar (esto ocurre si los datos estan en otros indirectos superiores).
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros_basico.c/liberador_punteros_datosRECURSIVO") "La recursiva del indirecto%d no tiene bloque de punteros, se omiten los %d bloques.\n",nivel_punteros,DatosOmitidos[nivel_punteros]); //Mensaje de error.
        *nprocesados = *nprocesados+DatosOmitidos[nivel_punteros]; //[BUGFIX] Ingresar todos los bloques de datos de esta capa como OMITIDOS y avanzar el seguimiento para el siguiente indirecto.
        return 0; //Finalizar prematuramente.
    }

    int BSmodus4 = NPUNTEROS; //Cantidad total de punteros contenidos dentro del bloque deseado.
    int retirarBloque = 0; //Si esta variable vale 1 (usado en el return de la recursiva), retira el bloque.
    unsigned int buffer[NPUNTEROS]; //Buffer de un bloque de datos exclusivamente en punteros (BLOCKSIZE/4).
    unsigned char bufferAux[BLOCKSIZE]; //El formato de un bloque 'totalmente' libre esta compuesto todo en "00" (todo en valor 0).
    memset(bufferAux, 0, BLOCKSIZE); //Establecer el bloque cada byte a '00' (todo en valor 0).

    bread(bloquepuntero, buffer); //Leer el bloque de punteros (indistintamente si es de datos o punteros, sigue usando esa funcionalidad).

    for(int i=0; i < BSmodus4 && *nprocesados < nblimite; i++) {
        if(nivel_punteros == 0) { //Si esta en modo "punteros a datos", libero bloques de datos.
            if(buffer[i] > 0 && *nprocesados >= bloqueminimo) retirarBloque = buffer[i]; //Si tiene datos en uso y esta fuera del margen de inclusion, marcar ese bloque de datos para liberar.
            *nprocesados = *nprocesados+1; //La cuenta de los bloques procesados (tamEnBytes) va en relación a bloques de DATOS.
        } else { //En caso de ser "punteros a punteros", delegar su tarea de liberar a la siguiente y espera a que se decida.
            if(buffer[i] > 0) retirarBloque = liberador_punteros_datosRECURSIVO(buffer[i], bloqueminimo, nivel_punteros-1, nblimite, nliberados, nprocesados); //Bajar nivel y delegar la tarea.
            else *nprocesados = *nprocesados+DatosOmitidos[nivel_punteros-1]; //[BUGFIX] Incluir en el anexo todos esos bloques omitidos para hacer avanzar el seguimiento de lo que tenga que liberar OMITIENDO lo previo.
        }
        if(retirarBloque > 0 && retirarBloque == buffer[i]) { //Si la funcion recursiva o de datos me retorna el valor de su bloque, retirar el bloque afectado.
            if(liberar_bloque(buffer[i]) >= 0) { //Esperar a tener permiso para liberar el bloque.
                if(PUNTERO_DEV && MODO_DEV) fprintf(stderr, MSG_NOTA("ficheros_basico.c/liberador_punteros_datosRECURSIVO") "Indirecto[%d]>>[%d], bloque '%d' anulado.\n",nivel_punteros,i,buffer[i]);
                buffer[i] = 0; //Indicar que ya no tiene datos en esa posición puesto que ha quedado libre.
                *nliberados = *nliberados+1; //Agregar en la cuenta de bloques liberados.
            }
            retirarBloque = 0; //Reiniciar el bloqueo de variable.
        }
    }
    if(memcmp(buffer, bufferAux, BLOCKSIZE) == 0) { //Mi bloque de punteros ha quedado vacio, he de avisar a la funcion invocada que me libere.
        //printf("El bloque '%d' del indirecto ha quedado huerfano, liberar el bloque desde afuera.\n",bloquepuntero); //Usado para ver si detecta o no bloques vacios (todo 0).
        return bloquepuntero; //Retornar su bloque asociado para que afuera se encarge de limpiar.
    }
    return 0;
}
