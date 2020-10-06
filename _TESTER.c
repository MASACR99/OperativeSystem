#include "directorios.h"
#include <stdarg.h>
//#include "ficheros_basico.h"

//Mensaje de excepciones, usado para regular mejor las funciones.
//MSG_TESTER es como el MSG cualquiera a parte del parentesis, tambien necesita el mensaje a lanzar. Incluye el salto de linea.
//TVAR resalta la cadena 'cual' de un color especifico, el precolor ha de ser una de las definiciones de COLOR ANSI del z_EXTRA.
#define MSG_TESTER(donde, contenido) COLOR_ROJO "<!> ('" donde "')" COLOR_NEGRO " \t " contenido "\n"
#define TVAR(precolor, cual) precolor cual COLOR_NEGRO

int amplitud;
const char *nombre_fichero;
unsigned int errores = 0; //Numero de errores que han dado lugar. Disponible a partir de la función 11 a más adelante (ficheros.c).

//El _TESTER es la zona de pruebas para probar de cerca las funciones implementadas.
//Al no usar _TESTER.h, siempre muestra un warning por funcion implicita.

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, MSG_FATAL("_TESTER.c/main") "_TESTER <nombre_fichero> <amplitud>\n");
        exit(EXIT_FAILURE);
    }
    nombre_fichero = argv[1];
    amplitud = atoi(argv[2]); //Hay que ponerlo en las funciones de prueba separadas, luego ya se verá si se le da uso o no.
    bmount(nombre_fichero);   //Si uso "bmountMK", se pondra en modo de crear el fichero (no usarlo, a menos que gestiones manualmente un mkfs [las primeras funciones]).
    // *****    *****    *****
    testMain13(argv[3]);

    return 0;
}

//Para lanzar errores de control del tester, aplicar el raiseTester como si fuera un printf, con la diferencia que tambien necesita un prefijo.
//raiseTester(MSG_TESTER("FUNCION USADA"," MENSAJE A DECLARAR ")); //Mensaje sin ninguna variable a mostrar.
//raiseTester(MSG_TESTER("FUNCION USADA"," MENSAJE A DECLARAR %d "), VARIABLE 1); //1 variable, el %d es un ejemplo.
//raiseTester(MSG_TESTER("FUNCION USADA"," MENSAJE A DECLARAR %d y su ejemplo2 %d "), VARIABLE 1, VARIABLE 2, ...); //Varias variables usadas.
void raiseTester(const char *mensaje, ...)
{
    va_list argptr;
    va_start(argptr, mensaje);
    vfprintf(stderr, mensaje, argptr);
    va_end(argptr);
    errores++;
}

int testMain1()
{
    printf(MSG_INFO(">>") " testMain1() 'manejo de los bloques' --> Disparado\n");
    char clon[] = "¿Qué és el Lorem Ipsum aqui?"; //sizeof = 32, block=31, OK.
    //char clon[] = "¿Qué és el Lorem Ipsum colega?";
    unsigned char buf[BLOCKSIZE];
    char ins[BLOCKSIZE];
    //char buf[BLOCKSIZE];
    //char rol[BLOCKSIZE];
    // *** ESTAS TRES LINEAS TENDRÁ SU UTILIDAD AL ESCRIBIR BLOQUES.
    memset(ins, 0, BLOCKSIZE);
    printf(MSG_VAR("-- ANCHURA --") "ins '%lu', clon '%lu', BLOCK '%lu'\n", sizeof ins, sizeof clon, sizeof BLOCKSIZEOF);
    strncpy(ins, clon, sizeof BLOCKSIZEOF); //Es importante dejar el BLOCKSIZE*8 con el -1, porque causa fallos de final de linea.
    //ins[6] = 't'; //En la columna X, se inserta el caracter.
    //buf[0] = 't'; //En la columna X, se inserta el caracter.
    //
    //Habilitar el ajuste "BLOCK_DEV" para accionar la visualización.
    bwrite(1, ins);
    bread(0, buf);
    bread(1, buf);
    bread(2, buf);
    //
    return 0;
}

int testMain2(int numblock)
{
    printf(MSG_INFO(">>") " testMain2() 'generar estructura INIT' --> Disparado\n");
    //tamMB(40);
    initSB(numblock, numblock / 4); //Ejemplo de superbloque
    printf(MSG_OK(">>") " initSB() <> resultados:\n");
    struct superbloque sb;
    bread(0, &sb);
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
    //printf(MSG_VAR("SB.padding") "%ld\n", sizeof(sb.padding));
    printf(MSG_VAR("info superbloque: sizeof") "%lu\n", sizeof(struct superbloque));
    printf(MSG_INFO(" ******* ******* ******* ******* ") "\n\n");
    initMB();
    printf(MSG_OK(">>") " initMB() <> resultados (nada aqui):\n");
    printf(MSG_INFO(" ******* ******* ******* ******* ") "\n\n");
    initAI();
    printf(MSG_OK(">>") " initAI() <> resultados:\n");
    printf(MSG_VAR("info inodo: blocksize") "%d\n", BLOCKSIZE);
    printf(MSG_VAR("info inodo: sizeof") "%lu\n", sizeof(struct inodo));
    /*struct inodo nodus[BLOCKSIZE/sizeof(struct inodo)];
    int aux = 0;
    for(int i = sb.posPrimerBloqueAI; i < sb.posUltimoBloqueAI; i++) {
        bread(i,nodus);
        for(int j; j < BLOCKSIZE/sizeof(struct inodo) && aux < sb.totInodos; j++) {
            //
        }
    }*/
    printf(MSG_INFO(" **FIN** ") " testMain2() 'generar estructura INIT'\n");
    return 0;
}

int testMain3()
{
    // La prueba 3 es un equivalente a "leer_sf", le doy un un fichero EXISTENTE y debe de revelar su contenido.
    printf(MSG_INFO(">>") " testMain3() 'mostrar la estructura' --> Disparado\n");
    printf(MSG_OK(">>") " initSB() <> resultados:\n");
    struct superbloque sb;
    bread(0, &sb);
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
    //printf(MSG_VAR("SB.padding") "%ld\n", sizeof(sb.padding));
    printf(MSG_INFO(" ******* ******* ******* ******* ") "\n");
    printf(MSG_OK(">>") " initMB() <> resultados\n");
    //for(int i=0; i <= 88195; i++) { escribir_bit(i,1); } //Usado para cazar el bug del mapa de bits. Inflexion de error hallada en "8191-8193"
    //for(int i = posSB; i <= sb.posUltimoBloqueDatos; i++) { //Mostrar el mapa de bits en su totalidad. Aviso, es conveniente redirigir la salida a un fichero.
    //    printf("[%d]: %hhu\t", i,leer_bit(i));
    //    if(i % 8 == 7) printf("\n");
    //}
    printf(MSG_VAR("leerBit - SB") "bloque: %d, estado bit: %d\n", posSB, leer_bit(posSB));
    printf(MSG_VAR("leerBit - InicioMB") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueMB, leer_bit(sb.posPrimerBloqueMB));
    printf(MSG_VAR("leerBit - UltimoMB") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueMB, leer_bit(sb.posUltimoBloqueMB));
    printf(MSG_VAR("leerBit - InicioAI") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueAI, leer_bit(sb.posPrimerBloqueAI));
    printf(MSG_VAR("leerBit - UltimoAI") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueAI, leer_bit(sb.posUltimoBloqueAI));
    printf(MSG_VAR("leerBit - InicioDatos") "bloque: %d, estado bit: %d\n", sb.posPrimerBloqueDatos, leer_bit(sb.posPrimerBloqueDatos));
    printf(MSG_VAR("leerBit - UltimoDatos") "bloque: %d, estado bit: %d\n", sb.posUltimoBloqueDatos, leer_bit(sb.posUltimoBloqueDatos));
    printf(MSG_INFO(" ******* ******* ******* ******* ") "\n");
    printf(MSG_OK(">>") " initAI() <> resultados:\n");
    struct inodo inodos[BLOCKSIZE / sizeof(struct inodo)];
    int siguienteInodo = sb.posPrimerInodoLibre;
    for (int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++)
    {
        bread(i, inodos);
        if (i < sb.posPrimerBloqueAI + 3 || i > sb.posUltimoBloqueAI - 3)
        {
            printf(MSG_NOTA("_TESTER/blocklook") "Bloque objetivo: %d [+%d]\n", i, i * BLOCKSIZE);
            if (i == sb.posPrimerBloqueAI + 2)
            {
                printf(MSG_NOTA("_TESTER/blocklook") " [OMITIR FRANJA] .  .  . \n");
            }
        }
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE) && siguienteInodo < sb.totInodos; j++)
        { //BUGFIX: totInodos
            if (MODO_DEV && (siguienteInodo <= 10 || siguienteInodo > sb.totInodos - 10))
            {
                if (siguienteInodo == sb.posInodoRaiz)
                {
                    printf(MSG_VAR("inodo %d [mod %d], tipo '%c' -- RAÍZ") "next pointer: %d\n", siguienteInodo, j, inodos[j].tipo, inodos[j].punterosDirectos[0]);
                }
                else
                {
                    printf(MSG_VAR("inodo %d [mod %d], tipo '%c'") "next pointer: %d\n", siguienteInodo, j, inodos[j].tipo, inodos[j].punterosDirectos[0]);
                }
            }
            siguienteInodo++;
        }
    }
    printf(MSG_INFO(" ******* ******* ******* ******* ") "\n");
    printf(MSG_OK(">>") " sizeof MISC <> resultados:\n");
    printf(MSG_VAR("info superbloque: sizeof") "%lu\n", sizeof(struct superbloque));
    printf(MSG_VAR("info inodo: blocksize") "%d\n", BLOCKSIZE);
    printf(MSG_VAR("info inodo: sizeof") "%lu\n", sizeof(struct inodo));
    printf(MSG_VAR("info: núm inodos por bloque [mod offset]") "%lu\n", BLOCKSIZE / sizeof(struct inodo));
    return 0;
}

int testMain4(int nbloque)
{
    //La prueba 4 es la base de leer/escribir "BIT" en el mapa de bits.
    //ES NECESARIO HABILITAR EL "bmountMK" para correr esta prueba paralela, no necesita mi_mkfs previo.
    //
    //Un BLOCKSIZE de 64, admite de 0 a 511 por cada bloque.
    //128 >> 1023 ||| 256 >> 2047 ||| 512 >> 4095 ||| "1024 >> 8191"
    //Esto quiere decir... permite guardar el octuple de bloques GENERICO por cada bloque de mapa de bits.
    //Atento, secciona el byte en 2 nibbles (0000 > 1111) en vez de (00000000 > 11111111).
    unsigned char ins[BLOCKSIZE];
    unsigned char leer[BLOCKSIZE];
    unsigned char mascara = 128; //Explicacion, para el byte "(128,64,32,16 || 8,4,2,1)".
    if (nbloque < 0)
    {                              //Para provocar un reset y tenga explicitamente 1 bloque, no necesita mi_mkfs.
        memset(ins, 0, BLOCKSIZE); //Bajo suposicion
        bwrite(0, ins);            //Genera el bloque 0, preferiblemente con un blocksize de 64 para verlo "resumido".
        return 0;
    }
    int posbit = nbloque % 8;  // 00 >> FF (byte entero, 2 nibbles)
    int posbyte = nbloque / 8; // Desplazamiento entre diferentes grupos de bytes enteros
    mascara >>= posbit;        // "128/2^posbit"  Nibble izquierda "0"0 (128,64,32,16) o derecha 0"0" (8,4,2,1).
    bread(0, ins);
    ins[posbyte] |= mascara; //El operador OR, asigna el bit 1 sobre el nibble ubicado relativamente en el bloque.
    if (nbloque == 11)
    {                             //Lo ponemos sobre 1 bloque especifico, porque empiezan todos con 0 y no haría nada si esta a cero, esto sirve para probar su funcionamiento.
        ins[posbyte] &= ~mascara; //El operador AND, asigna el bit 0 sobre el nibble ubicado.
    }
    bwrite(0, ins);
    bread(0, leer);
    //
    //Zona de lectura especifica.
    printf(MSG_INFO("leer mapa de bits") "{ '1º grupo de 2 bytes, los primeros'");
    for (int i = 0; i < 32; i++)
    {
        mascara = 128;
        posbit = i % 8;
        posbyte = (i / 8);        //%BLOCKSIZE; //Tendría utilidad si hubiese 2 o más bloques, cuyo caso se ignora.
        mascara >>= posbit;       // ' 128/2^posbit ' (desplazamiento binario a la derecha)
        mascara &= leer[posbyte]; //Prueba con un AND si el 128/2^posbit es valido.
        mascara >>= (7 - posbit); //Si devuelve su valor '128/2^posbit' (AND aceptado), hará el desplazamiento de bits (binario) hacia a la derecha
        if (i % 8 == 0)
        {
            printf(" }\n {");
        }
        printf("  [%d]: %hhu", i, mascara);
    }
    printf(" }\n");
    return 0;
    //int trasteria = 256; //Valor de ejemplo inicial.
    //trasteria >>= 2; // 256/2^2 (4) = 64
    //trasteria >>= 1; // 256/2^1 (2) = 32
}

int testMain5(int nbloque)
{
    //La prueba 5 usa el leer/escribir "BIT" del mapa de bits en ficheros_basico y una estructura mi_mkfs previa.
    //Esta prueba se enfoca en especial para el liberar/reservar_bloque.
    if (nbloque < 0)
    {                       //Para provocar que se agrege cosas, sin repetir inutilmente.
        escribir_bit(1, 1); //+64 en nibble 1
        escribir_bit(2, 1); //+32
        escribir_bit(5, 1); //+4
        escribir_bit(8, 1); //+128 en nibble 2
        if (nbloque % 2 == 0)
        {                       //Para probar que puede desactivarse, usar un bloque PAR negativo para accionarlo.
            escribir_bit(1, 0); //-64, deberia de ser 36 (32,4) en nibble 1, y 128 en nibble 2.
        }
        return 0;
    }
    int valorBit = leer_bit(nbloque);
    printf(MSG_VAR("lectura main5") "bloque: %d, en estado bit: %d\n", nbloque, valorBit);
    return 0;
}

int testMain6(int nbloque)
{
    //La prueba 6 usa el liberar/reservar_bloque, en conjunto, usa la prueba 5 para VERIFICAR...
    if (nbloque < 0)
    { //Cuando le estipule el valor negativo, muestrame el sistema de ficheros (test 3).
        return testMain3();
    }
    //Esta parte es para hacer la prueba de liberar un bloque y ver si funciona.
    liberar_bloque(nbloque);
    exit(EXIT_SUCCESS);
    liberar_bloque(2009);
    liberar_bloque(2019);
    liberar_bloque(2019);
    int valorBit = reservar_bloque();
    if (valorBit >= 0)
        testMain5(valorBit);
    valorBit = reservar_bloque();
    if (valorBit >= 0)
        testMain5(valorBit);
    valorBit = reservar_bloque();
    if (valorBit >= 0)
        testMain5(valorBit);
    return 0;
}

int testMain7()
{
    //Prueba para escribir el directorio raíz y ver si las funciones "escribir_inodo", "leer_inodo"
    //y "reservar_inodo" funcionan correctamente.
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    struct inodo inodo;

    //reservamos el directorio raíz: será un directorio y tendrá permisos de lectura y escritura
    int buscarInodo = reservar_inodo('d', 7);

    //Ahora leemos los datos del directorio raíz y los mostramos por pantalla
    leer_inodo(buscarInodo, &inodo); //En vez del 0, conviene probar las otras ejecucciones, por eso esta "buscarInodo"
    printf(MSG_VAR("_TESTER/testMain7") "ninodo: %d\n", buscarInodo);

    printf(MSG_VAR("_TESTER/testMain7") "tipo: %c\n", inodo.tipo);
    printf(MSG_VAR("_TESTER/testMain7") "permisos: %d\n", inodo.permisos);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf(MSG_VAR("_TESTER/testMain7") "ID: %d ATIME: %s MTIME: %s CTIME: %s\n", 0, atime, mtime, ctime);
    printf(MSG_VAR("_TESTER/testMain7") "nlinks: %d\n", inodo.nlinks);
    printf(MSG_VAR("_TESTER/testMain7") "tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf(MSG_VAR("_TESTER/testMain7") "numBloquesOcupados: %d\n", inodo.numBloquesOcupados);

    return buscarInodo;
}

int testMain8(int lugar)
{
    //Prueba la funcion de traducir el bloque logico (obtener_indice) y su grado sobre (in)directo(s).
    //Los fallos menores ya estan corregidos, esta funcion ya esta completamente operativa.
    printf(MSG_NOTA("_TESTER/testMain8") "amplitudPersonalizada: %d\n", lugar);                 //Mostrar cual hace referencia para que sea visible.
    printf(MSG_VAR("_TESTER/testMain8") "indice directo: %d\n", obtener_indice(lugar, 0));      //Directo
    printf(MSG_VAR("_TESTER/testMain8") "indice resultante 1: %d\n", obtener_indice(lugar, 1)); //Indirecto0
    printf(MSG_VAR("_TESTER/testMain8") "indice resultante 2: %d\n", obtener_indice(lugar, 2)); //Indirecto1
    printf(MSG_VAR("_TESTER/testMain8") "indice resultante 3: %d\n", obtener_indice(lugar, 3)); //Indirecto2
    return 0;
}

int testMain9(int lugar)
{
    //Prueba en la funcion "traducir_bloque_inodo", a su vez... afecta a "obtener_nRangoBL".
    struct superbloque sb;
    struct inodo inodo;
    bread(0, &sb);
    leer_inodo(sb.posInodoRaiz, &inodo); //Prueba si existe el inodo raíz.
    if (inodo.tipo == 'l')
    { //No esta operativo, iniciar la reserva.
        int algo = reservar_inodo('d', 7);
        printf(MSG_INFO("_TESTER/testMain9") "Designado el inodo %d como directorio\n", algo);
        //} else { //Iniciado previamente, mantener.
        //    printf(MSG_VAR("_TESTER/testMain8") "inodo %d ya presente, tipo: %c\n", sb.posInodoRaiz, inodo.tipo);
    }
    printf(MSG_NOTA("_TESTER/testMain9") "amplitudPersonalizada: %d\n", lugar); //Mostrar cual hace referencia para que sea visible.
    //La de abajo prueba si se reserva un bloque de datos.
    //POSIBLE BUG: Cuando "lugar" (nblogico) es >=12, experimenta el error "*** stack smashing detected ***: <unknown> terminated", dicen que este error ocurre por overflow en algún array de las variables.
    //ARREGLO PROVISIONAL 'buffer[blocksize]' en vez de 'buffer[indice]', conviene inspeccionar nuevamente su estructura.
    printf(MSG_VAR("_TESTER/testMain9") "Valor ptr final al procesar: %d\n", traducir_bloque_inodo(sb.posInodoRaiz, lugar, 1));
    leer_inodo(sb.posInodoRaiz, &inodo);                                                                                    //Vuelve a leer el valor del inodo, debería de reflejarse los bloques ocupados.
    printf(MSG_AVISO("_TESTER/testMain9") "inodo %d, numBloquesOcupados: %d\n", sb.posInodoRaiz, inodo.numBloquesOcupados); //Mostrar los bloques ocupados
    return 0;
}

int testMain10(int lugar)
{
    //Esta prueba lo último de ficheros_basico, el de liberar inodo y los bloques.
    //Tanto el inodo raíz como intacto, no han de ser liberados. La victima es quien ha de variar.
    struct superbloque sb;
    struct inodo inodoRaiz;
    struct inodo inodoVictima;
    struct inodo inodoIntacto;
    bread(0, &sb);
    int victima;          //Núm inodo ubicado
    int intacto;          //Núm inodo ubicado
    int numinodosprevios; //Variable de comprobación de inodos libres tras liberar la victima.
    //preasignar los 3 primeros inodos.
    reservar_inodo('d', 7);           //0, raíz, ocupara el bloque que le especifique el argumento.
    victima = reservar_inodo('d', 6); //1, la que se modifica.
    intacto = reservar_inodo('d', 4); //2, la no modificada.
    //      OCUPAR LOS BLOQUES EN LOS INODOS PARA HACER LA PRUEBA.
    //traducir_bloque_inodo(sb.posInodoRaiz,lugar,1);
    traducir_bloque_inodo(victima, 8, 1);
    traducir_bloque_inodo(victima, 204, 1);
    traducir_bloque_inodo(victima, 30004, 1);
    traducir_bloque_inodo(victima, 400004, 1);
    traducir_bloque_inodo(victima, 16843019, 1);
    //traducir_bloque_inodo(intacto,400004,1);
    printf(MSG_NOTA("OJO...") "Fin de reservar bloques en los inodos; fase de prueba de liberar.\n");
    leer_inodo(sb.posInodoRaiz, &inodoRaiz);
    leer_inodo(victima, &inodoVictima);
    leer_inodo(intacto, &inodoIntacto);
    bread(0, &sb);
    printf(MSG_INFO("_TESTER/testMain10") "[PRE] inodo raíz '%d', numBloquesOcupados: '%d', tipo: '%c'\n", sb.posInodoRaiz, inodoRaiz.numBloquesOcupados, inodoRaiz.tipo);               //Mostrar los bloques ocupados
    printf(MSG_AVISO("_TESTER/testMain10<!IMPORTANTE") "[PRE] inodo victima '%d', numBloquesOcupados: '%d', tipo: '%c'\n", victima, inodoVictima.numBloquesOcupados, inodoVictima.tipo); //Mostrar los bloques ocupados
    printf(MSG_INFO("_TESTER/testMain10") "[PRE] inodo no tocado '%d', numBloquesOcupados: '%d', tipo: '%c'\n", intacto, inodoIntacto.numBloquesOcupados, inodoIntacto.tipo);            //Mostrar los bloques ocupados
    printf(MSG_VAR("sb.posPrimerInodoLibre") "[PRE] Valor actual: '%d'\n", sb.posPrimerInodoLibre);
    printf(MSG_VAR("sb.cantInodosLibres") "[PRE] Valor actual: '%d'\n\n", sb.cantInodosLibres);
    printf(MSG_INFO("******* ******* ******* ******* ") "Delimitador, ahora se procede a liberar el inodo.\n\n");
    numinodosprevios = sb.cantInodosLibres + 1;
    liberar_inodo(victima);
    printf("\n\n\t\tRESULTADOS AL LIBERAR:\n");
    // *****    *****    *****      --- Volver a leer los datos actualizados tras el cambio.
    bread(0, &sb);
    leer_inodo(sb.posInodoRaiz, &inodoRaiz);
    leer_inodo(victima, &inodoVictima);
    leer_inodo(intacto, &inodoIntacto);
    // *****    *****    *****
    printf(MSG_INFO("_TESTER/testMain10") "[POST] inodo raíz '%d', numBloquesOcupados: '%d', tipo: '%c'\n", sb.posInodoRaiz, inodoRaiz.numBloquesOcupados, inodoRaiz.tipo); //Mostrar los bloques ocupados
    if (inodoVictima.numBloquesOcupados != 0 || inodoVictima.tipo != 'l')
        printf(MSG_ERROR("_TESTER/testMain10") "[POST] inodo victima (FALLADA) '%d', numBloquesOcupados: '%d' (HA DE APARECER '0'), tipo: '%c' (HA DE SER 'l')\n", victima, inodoVictima.numBloquesOcupados, inodoVictima.tipo); //Mostrar los bloques ocupados, si aparece esta línea, la función del liberar tiene errores.
    else
        printf(MSG_OK("_TESTER/testMain10") "[POST] inodo victima '%d', numBloquesOcupados: '%d', tipo: '%c'\n", victima, inodoVictima.numBloquesOcupados, inodoVictima.tipo); //Mostrar los bloques ocupados
    printf(MSG_INFO("_TESTER/testMain10") "[POST] inodo no tocado '%d', numBloquesOcupados: '%d', tipo: '%c'\n", intacto, inodoIntacto.numBloquesOcupados, inodoIntacto.tipo); //Mostrar los bloques ocupados
    if (sb.posPrimerInodoLibre != victima)
        printf(MSG_AVISO("sb.posPrimerInodoLibre") "[POST] Valor actualizado: '%d' (TENIA QUE VALER '%d')\n", sb.posPrimerInodoLibre, victima);
    else
        printf(MSG_VAR("sb.posPrimerInodoLibre") "[POST] Valor actualizado: '%d'\n", sb.posPrimerInodoLibre);
    if (numinodosprevios != sb.cantInodosLibres)
        printf(MSG_AVISO("sb.cantInodosLibres") "[POST] Valor actualizado: '%d' (TENIA QUE VALER '%d')\n", sb.cantInodosLibres, numinodosprevios);
    else
        printf(MSG_VAR("sb.cantInodosLibres") "[POST] Valor actualizado: '%d'\n", sb.cantInodosLibres);
    return 0;
}

//      SECCION PARA FICHEROS.C
int testMain11(int lugar, char *algo)
{ //La amplitud del argumento hará de offset en un futuro, por ahora es estatico.
    //Esta funcion prueba el I/O de un String en write_f y read_f.
    struct superbloque sb;
    struct inodo inodo;
    bread(0, &sb);
    leer_inodo(sb.posInodoRaiz, &inodo); //Prueba si existe el inodo raíz.
    if (inodo.tipo == 'l')
    { //No esta operativo, iniciar la reserva.
        int ignorar = reservar_inodo('d', 7);
        printf(MSG_INFO("_TESTER/testMain11") "Designado el inodo %d como directorio\n", ignorar);
    }

    char solapar[] = TVAR(COLOR_AZUL, "¿ola que ase?, esta cadena tenía que aparecer o ve a saber que ase el write_f cuando este sólo ante el Lorem Ipsum") COLOR_NEGRO "\n"; //Ha de solapar esta cadena.
    unsigned int longi = strlen(algo);
    unsigned int solapable = strlen(solapar);
    unsigned int byteoffset = 100; //Offset prefijado.
    //char algo[] = "¿ola que ase?, esta cadena tenía que aparecer o ve a saber que ase el write_f cuando este sólo ante el Lorem Ipsum";
    char *salida = malloc(BLOCKSIZE);
    //unsigned char *salida = malloc(BLOCKSIZE);
    memset(salida, 0, strlen(salida));
    printf(COLOR_AMARILLO "\nTamaño bytes String: %d  (+%d offset sometido)\n\n" COLOR_NEGRO, longi, byteoffset);
    int escritos = mi_write_f(sb.posInodoRaiz, algo, byteoffset, longi);   //Por ahora, el offset (3º argumento) funciona cuando es exactamente 0 o use sólo 1 bloque.
    mi_write_f(sb.posInodoRaiz, solapar, byteoffset + 910, solapable - 1); // !! SOLAPAR parte de los datos que escribir con esta otra cadena.
    int leidos = mi_read_f(sb.posInodoRaiz, salida, byteoffset, longi);    //Por ahora, el offset (3º argumento) funciona cuando es exactamente 0 o use sólo 1 bloque.
    if (escritos != longi)
        raiseTester(MSG_TESTER("mi_write_f", "Se escribio sólo " TVAR(COLOR_AMARILLO, "'%d'") " nbytes de los " TVAR(COLOR_ROJO, "%d") ", repasar porque se deja algunos"), escritos, longi); //Alertar si difiere los bytes escritos.
    if (leidos != longi)
        raiseTester(MSG_TESTER("mi_read_f", "TEXTO GENERADO (" TVAR(COLOR_AMARILLO, "%d") " bytes leidos, pero tenia que ser " TVAR(COLOR_ROJO, "%d") ", revisar porque difiere el tamaño):"), leidos, longi); //Alertar cuando la lectura no coincida.
    else
        printf("TEXTO GENERADO (" TVAR(COLOR_VERDE, "'%d' [válido]") " bytes leidos, parece que funciona): \n", leidos); //Este mensaje aparecerá cuando los bytes leidos coincida con la detectada.
    printf("'%s'\n", salida);
    return 0;
}

int testRecur()
{ //Sistema de pruebas para probar el funcionamiento del liberar recursivo.
    //Empieza por escribir bloques del inodo 1 y luego se intenta truncar, la solucion tendría que igualarse frente al iterativo.
    struct inodo inodo;
    char algo[] = "¿ola que ase?, esta cadena es la que se escribe y según donde la ejecute el truncar prevalece o se ira al garete, de parte del Lorem Ipsum pará inflar."; //Lo que inserta en los bloques.
    int ninodo = reservar_inodo('f', 6);
    int anchuraSizeof = strlen(algo);
    int retornado;
    int bloquesEsperados;

    if(errores > 0) return -1;
    //****** FASE 1 --- Punteros DIRECTOS, todos ocupados.
    for(int i = 0; i < 12; i++) {
        mi_write_f(ninodo, algo, i*BLOCKSIZE, anchuraSizeof);
        //printf("escrito '%d', en el margen '%d'\n",anchuraSizeof, i*BLOCKSIZE);
    }
    leer_inodo(ninodo,&inodo);
    bloquesEsperados = inodo.numBloquesOcupados;
    printf("[PRE-FASE 1]: tamEnBytesLog " TVAR(COLOR_CYAN,"%d") " usando " TVAR(COLOR_CYAN,"%d") " bloques.\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    retornado = mi_truncar_f(ninodo,0);
    leer_inodo(ninodo,&inodo);
    if(retornado != bloquesEsperados || inodo.tamEnBytesLog != 0) { //Comprueba si se libero todos los libres y no deje residuo.
        raiseTester(MSG_TESTER("testRecur/fase1", "[POST-FASE 1]: tamEnBytesLog " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " usando " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " bloques.\nLa funcion principal no llega a liberar los directos.\n"),inodo.tamEnBytesLog,0,retornado,bloquesEsperados);
        return -1;
    }
    printf("[POST-FASE 1]: tamEnBytesLog " TVAR(COLOR_VERDE,"%d") " usando " TVAR(COLOR_VERDE,"%d") " bloques.\n\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    
    //****** FASE 2 --- Punteros DIRECTOS, respetar el offset inicial. Mantener 1 bloque ocupado aunque no tenga datos (se escribieron desde el byte 20), pero no se puede liberar el bloque porque tamBytes aun no es 0.
    mi_write_f(ninodo, algo, 20, anchuraSizeof);
    leer_inodo(ninodo,&inodo);
    printf("[PRE-FASE 2]: tamEnBytesLog " TVAR(COLOR_CYAN,"%d") " usando " TVAR(COLOR_CYAN,"%d") " bloques.\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    leer_inodo(ninodo,&inodo);
    bloquesEsperados = 0; //No se libera ningún bloque.
    retornado = mi_truncar_f(ninodo,10); //Deberia quedar vacio el contenido pero ha de mantener ese bloque ocupado (no liberar).
    leer_inodo(ninodo,&inodo);
    if(retornado != bloquesEsperados || inodo.tamEnBytesLog != 10) { //Comprueba si se libero todos los libres y no deje residuo.
        raiseTester(MSG_TESTER("testRecur/fase2", "[POST-FASE 2]: tamEnBytesLog " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " usando " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " bloques.\nRevisar la funcion principal acerca de los directos.\n"),inodo.tamEnBytesLog,10,retornado,bloquesEsperados);
        return -1;
    }
    printf("[POST-FASE 2]: tamEnBytesLog " TVAR(COLOR_VERDE,"%d") " usando " TVAR(COLOR_VERDE,"%d") " bloques.\n\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    
    //****** FASE 3 --- Punteros INDIRECTOS 0, todos ocupados en directos y parcialmente el indirecto 0 y 1.
    for(int i = 0; i < 26; i++) {
        mi_write_f(ninodo, algo, i*BLOCKSIZE, anchuraSizeof);
    }
    mi_write_f(ninodo, algo, 272*BLOCKSIZE, anchuraSizeof); //Para tratar de ponerle cuerda
    leer_inodo(ninodo,&inodo);
    bloquesEsperados = inodo.numBloquesOcupados;
    printf("[PRE-FASE 3]: tamEnBytesLog " TVAR(COLOR_CYAN,"%d") " usando " TVAR(COLOR_CYAN,"%d") " bloques.\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    retornado = mi_truncar_f(ninodo,0);
    leer_inodo(ninodo,&inodo);
    if(retornado != bloquesEsperados || inodo.tamEnBytesLog != 0) { //Comprueba si se libero todos los libres y no deje residuo.
        raiseTester(MSG_TESTER("testRecur/fase3", "[POST-FASE 3]: tamEnBytesLog " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " usando " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " bloques.\nRevisar la recursiva del indirecto0, no llega a liberar los bloques de datos que estan contenidos.\n"),inodo.tamEnBytesLog,0,retornado,bloquesEsperados);
        return -1;
    }
    printf("[POST-FASE 3]: tamEnBytesLog " TVAR(COLOR_VERDE,"%d") " usando " TVAR(COLOR_VERDE,"%d") " bloques.\n\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    
    //****** FASE 4 --- Punteros INDIRECTOS 0, mantener los dos primeros bloques del indirecto0, omite los directos.
    //Escribir y llenar el indirecto 0 con bloques de datos, luego al truncar, ha de despreciar 253 bloques de los 255; tiene que conservar el bloque indirecto 0 y los dos primeros del indirecto.
    for(int i = 12; i < 268; i++) {
        //printf("i '%d', offset '%d'\n",i,i*BLOCKSIZE);
        mi_write_f(ninodo, algo, i*BLOCKSIZE, anchuraSizeof); //    !!!!    ¿¿¿Error en el mi_write_f????
    }
    //return -1;
    leer_inodo(ninodo,&inodo);
    bloquesEsperados = inodo.numBloquesOcupados-3; //El bloque punteros indirecto0 y los dos bloques de datos del indirecto0.
    printf("[PRE-FASE 4]: tamEnBytesLog " TVAR(COLOR_CYAN,"%d") " usando " TVAR(COLOR_CYAN,"%d") " bloques.\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);
    retornado = mi_truncar_f(ninodo,14336); //Mantiene los directos, y los 2 primeros indirectos0; se libera "253" bloques.
    leer_inodo(ninodo,&inodo);
    if(retornado != bloquesEsperados || inodo.tamEnBytesLog != 14336) { //Comprueba si se libero todos los libres y no deje residuo.
        raiseTester(MSG_TESTER("testRecur/fase4", "[POST-FASE 4]: tamEnBytesLog " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " usando " TVAR(COLOR_AMARILLO,"%d") "/" TVAR(COLOR_ROJO,"%d") " bloques.\nRevisar la recursiva del indirecto0, el concepto de inclusion ha de mantener los 2 primeros indirectos.\n"),inodo.tamEnBytesLog,14336,retornado,bloquesEsperados);
        return -1;
    }
    printf("[POST-FASE 4]: tamEnBytesLog " TVAR(COLOR_VERDE,"%d") " usando " TVAR(COLOR_VERDE,"%d") " bloques.\n\n",inodo.tamEnBytesLog,inodo.numBloquesOcupados);

    //ME FALTA TERMINAR EL MARCO DE PRUEBAS.
    return 0;
}

int testSellosPermiso(int muestraEjemplar) {
    DEFINIR_SELLO_RWX; //Importar del z_EXTRA la variable array "SELLO_RWX". Es un string usado para el print.
    printf("Para el caso de los sellos como este " TVAR(COLOR_VERDE, "%s") " por ejemplo\n", SELLO_RWX[muestraEjemplar]); //Ejemplo de uso, usando el 2º argumento 'Amplitud' para indicar que parte del array se accede.
    return 0;
}

int testMain12(const char* camino)
{
    // Programa de test para probar la funcion extraer_camino (funciona correctamente)
    char *inicial, *final, *tipo;

    inicial = malloc(BLOCKSIZE/4);
    final = malloc(BLOCKSIZE/4);
    tipo = malloc(BLOCKSIZE/4);

    if (extraer_camino(camino, inicial, final, tipo) == -1)
    {
        printf(MSG_ERROR("_TESTER.c") "Error al extraer camino.\n");
        return -1;
    }

    printf(MSG_OK("_TESTER.c") "Camino extraido con éxito.\n");
    return 0;
}

int testMain13(const char* ruta)
{
    unsigned int p_entrada = 0,p_inodo = 0,p_inodo_dir = 0;
    char reservar;
    unsigned char permisos;

    buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada,'1', (unsigned char)6);
    printf("\n\n");
    printf("pInodoDir '%d', pInodo '%d', pEntrada '%d'\n",p_inodo_dir,p_inodo,p_entrada);

    return 0;
}