#include "ficheros.h"

//Escribe una cadena incluso una larga (la prueba segura fue hasta 110k lineas) sobre un inodo especifico.
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    unsigned int primerBL, ultimoBL, desplazamiento, desplazamiento2, BF, bytes_escritos = 0;
    int longi = strlen(buf_original); //Bugfix, si el buf_original es "" (tamaño 0) provoca errores internos, comprueba previamente si tiene algo escrito.
    unsigned char buffer[BLOCKSIZE];
    struct inodo inodo_lectura;

    leer_inodo(ninodo, &inodo_lectura); //Recuperar el estado del inodo.
    if ((inodo_lectura.permisos & 2) != 2) //Usa un operador binario para ver si contiene el valor del '2' (X1X); si el inodo no tiene permisos de escritura este finaliza con error.
    {
        fprintf(stderr, MSG_ERROR("ficheros.c/mi_write_f") "El inodo %d no tiene permisos de escritura (actualmente: %d).\n", ninodo, inodo_lectura.permisos);
        return -1;
    }
    if(longi <= 0) { //Bugfix: Abarcaba todos los inodos y hasta incluso generaba de ficticios cuando el texto (buf_original) estaba vacio.
        fprintf(stderr, MSG_ERROR("ficheros.c/mi_write_f") "El texto insertado esta vacio.\n");
        return -1;
    }
    //Si llegamos aquí, tenemos permisos de escritura sobre el inodo.
    primerBL = offset / BLOCKSIZE; //Determina el primer bloque donde se va a escribir, ponderado hacia abajo.
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; //El último bloque donde se va a escribir, acorde al número de bytes pedidos para escribir.
    memset(buffer, 0, BLOCKSIZE); //Buffer de lectura del bread para conservar los datos ya existentes y se solape únicamente los desplazamientos junto con su amplitud.
    if (primerBL == ultimoBL)
    { //Si sólo es un único bloque a escribir...
        //Debido a que es posible que el buffer no ocupe el bloque de datos entero, hay que guardar la porcion de buffer que no se va a escribir para no perder coherencia en los datos.
        //Para ello, tendremos que calcular el desplazamiento dentro del buffer y escribir en la posición deseada.

        desplazamiento = offset % BLOCKSIZE; //Cálculo del margen inicial.
        //Obtenemos el bloque físico, lo leemos y copiamos la información en el buffer en la posicion obtenida en la variable desplazamiento*/
        #ifdef ENTREGA_3
        mi_waitSem();
        #endif
        BF = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtener el bloque de datos a escribir, es posible encontrarse con uno ya existente.
        #ifdef ENTREGA_3
        mi_signalSem();
        #endif
        bread(BF, buffer); //Volcar el contenido previo.
        memcpy(buffer + desplazamiento, buf_original, nbytes); //Solapa una porción (o todo) del bloque de datos con la cadena que quería escribirse.
        bwrite(BF, buffer); //Actualiza el contenido del bloque volcado, solapado con la porción que se quiere escribir.
        bytes_escritos = nbytes; //Recuento de bytes que fueron escritos.
    }
    else
    { //Varios bloques se van a escribirse (2 o más)
        for (unsigned int i = primerBL; i <= ultimoBL; i++)
        { //Bloques a escribirse, acotados y sucesivos.
            if (i == primerBL)
            { //Primer bloque a escribirse.
                #ifdef ENTREGA_3
                mi_waitSem(); //traducir_bloque_inodo con reservar 1 es critico
                #endif
                BF = traducir_bloque_inodo(ninodo, primerBL, 1); //Obtener el bloque de datos a escribir, es posible encontrarse con uno ya existente.
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
                bread(BF, buffer); //Volcar el contenido previo.
                desplazamiento = offset % BLOCKSIZE; //Margen inicial dentro del bloque.
                memcpy(buffer + desplazamiento, buf_original, BLOCKSIZE - desplazamiento); //Solapa una porción (o todo) del bloque de datos con la cadena que quería escribirse inicialmente.
                bwrite(BF, buffer); //Actualiza el contenido del bloque volcado, solapado con la porción que se quiere escribir.
                bytes_escritos += (BLOCKSIZE - desplazamiento); //Recuento de bytes que fueron escritos desde el margen inicial.
            }
            else if (i == ultimoBL)
            { //Último bloque a escribirse.
                #ifdef ENTREGA_3
                mi_waitSem(); //traducir_bloque_inodo con reservar 1 es critico
                #endif
                BF = traducir_bloque_inodo(ninodo, ultimoBL, 1); //Obtener el bloque de datos a escribir, es posible encontrarse con uno ya existente.
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
                bread(BF, buffer); //Volcar el contenido previo.
                desplazamiento = offset % BLOCKSIZE;
                desplazamiento2 = nbytes-bytes_escritos; //Posible BUGFIX para incluso con o sin OFFSET. Margen restante para escribir.
                memcpy(buffer, buf_original + (BLOCKSIZE - desplazamiento) + (ultimoBL - primerBL - 1) * BLOCKSIZE, desplazamiento2 + 1); //Solapa una porción (o todo) del bloque de datos con la cadena que quería escribirse al final.
                bwrite(BF, buffer); //Actualiza el contenido del bloque volcado, solapado con la porción que se quiere escribir.
                bytes_escritos += (desplazamiento2); //Recuento de bytes que fueron escritos. (anteriormente, desplazamiento2+1)
            }
            else
            { //Uno o varios bloques intermedios, estos ocupan siempre bloques enteros.
                #ifdef ENTREGA_3
                mi_waitSem(); //traducir_bloque_inodo con reservar 1 es critico
                #endif
                BF = traducir_bloque_inodo(ninodo, i, 1); //Obtener el bloque de datos a escribir, es posible encontrarse con uno ya existente.
                #ifdef ENTREGA_3
                mi_signalSem();
                #endif
                bwrite(BF, buf_original + (BLOCKSIZE - desplazamiento) + (i - primerBL - 1) * BLOCKSIZE); //Reemplazar todo el contenido del bloque, no es necesario volcar porque afecta a todo el bloque y no deja nada previo.
                bytes_escritos += BLOCKSIZE; //Siempre suma el BLOCKSIZE (todo el bloque)
            }
        }
    }
    //Ahora toca actualizar toda la metainformacion del inodo.
    #ifdef ENTREGA_3
    mi_waitSem();
    #endif
    leer_inodo(ninodo, &inodo_lectura); //Recuperar estado del inodo.
    if (inodo_lectura.tamEnBytesLog < offset + bytes_escritos)
    { //Comprobar si la cantidad actual de bytes escritos (junto con su margen) es mayor que la previa. Si es así, designa el nuevo tamaño de bytes.
        //Solo actualizamos el campo de tamEnBytesLog si y solo si hemos escrito más allá del final del fichero.
        inodo_lectura.tamEnBytesLog = offset+bytes_escritos; //Tamaño nuevo y mayor que la de antes. (Faltaba ubicar el offset, porque sólo registraba los bytes que escribia, no la omision)
        inodo_lectura.ctime = time(NULL); //Reinicia la fecha de cuando fue creado.
    }
    inodo_lectura.mtime = time(NULL); //Reinicia el campo de fecha cuando fue modificado.
    escribir_inodo(ninodo, inodo_lectura); //Actualiza el estado del inodo.
    #ifdef ENTREGA_3
    mi_signalSem();
    #endif
    if(INODO_DEV) fprintf(stderr, MSG_VAR("ficheros.c/mi_write_f") "La cantidad de bytes escritos es: %d (+%d offset)\n", bytes_escritos, offset); //Si me lo piden explicitamente, le muestro donde ha tenido lugar.
    return bytes_escritos; //Retorna la cantidad de bytes que fueron escritos del inodo.
}

//Leer una cadena incluso una larga (la prueba segura fue hasta 110k lineas) sobre un inodo especifico.
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    //IMPORTANTE: ESTA FUNCION DEBE RECIBIR "*buf_original" inicializado a 0's porque podría terminar habiendo basura después de la ejecución de esta función.
    unsigned int primerBL = 0, ultimoBL = 0, desplazamiento = 0, desplazamiento2 = 0, bytes_leidos = 0;
    int BF = 0;
    unsigned char buffer[BLOCKSIZE];
    struct inodo inodo_lectura;
    memset(buffer, 0, BLOCKSIZE); //Limpia el buffer, podría tener problemas si no se limpia previamente.
    leer_inodo(ninodo, &inodo_lectura); //Recuperar el estado del inodo.
    if ((inodo_lectura.permisos & 4) != 4) //Usa un operador binario para ver si contiene el valor del '4' (1XX); si el inodo no tiene permisos de lectura este finaliza con error.
    {
        fprintf(stderr, MSG_ERROR("ficheros.c/mi_read_f") "El inodo %d no tiene permisos de lectura (actualmente %d).\n", ninodo, inodo_lectura.permisos);
        return -1;
    }
    //Ahora miramos si intentamos leer más allá del fichero o si intentamos leer más bytes de los que el fichero realmenmte ocupa.
    if(inodo_lectura.tamEnBytesLog <= 0) { //Bugfix: La función nunca finalizaba si no había nada escrito.
        fprintf(stderr, MSG_ERROR("ficheros.c/mi_read_f") "En el inodo %d no hay nada escrito, imposible de leer.\n",ninodo);
        return -1;
    }
    if (inodo_lectura.tamEnBytesLog < offset)
    { //Si mi offset inicial supera del limite escrito, siempre va a leer un contenido vacio, guste o no. Es decir, trata de leer más allá de lo que contiene en los datos del inodo.
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros.c/mi_read_f") "Se intenta leer más allá del final del fichero (tiene escrito hasta '%d' byte(s), baja el offset).\n", inodo_lectura.tamEnBytesLog);
        return 0; //Vale 0 lo que contiene (porque no hay nada más allá de lo ecrito).
    }
    if (offset + nbytes > inodo_lectura.tamEnBytesLog)
    { //Cuando puede leer almenos algo pero mi longitud excede de lo escrito, lo acoto al EOF.
        nbytes = inodo_lectura.tamEnBytesLog - offset;
    }
    //Si llegamos aquí, tenemos permisos de lectura sobre el inodo.
    primerBL = offset / BLOCKSIZE; //Determina el primer bloque donde se va a leer, ponderado hacia abajo.
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; //El último bloque donde se va a leer, acorde al número de bytes pedidos para hacerlo.
    if (primerBL == ultimoBL)
    { //Si sólo es un único bloque que tiene que leerse...
        desplazamiento = offset % BLOCKSIZE; //Cálculo del margen inicial en leer.
        //Obtenemos el bloque físico, lo leemos y copiamos la información en el buffer en la posicion obtenida en la variable desplazamiento.
        BF = traducir_bloque_inodo(ninodo, primerBL, 0); //Obtener el bloque de datos a leer por recorrido de punteros de nivel.
        if (BF != -1)
        {
            bread(BF, buffer); //Leer el contenido previo.
            //memcpy(buf_original + desplazamiento, buffer, nbytes); //Tenia fallos. (El offset no es el que ha de mostrar, es el que ha leido del bloque. Previamente, esta era la linea.)
            memcpy(buf_original, buffer + desplazamiento, nbytes); //Habia que hacer offset del buffer, luego la cantidad de bytes (que fue detraida)
        }
        bytes_leidos += nbytes; //Previamente... ahora funciona porque tamBytes ya procesa el offset del write.
    }
    else
    { //Varios bloques se van a leerse (2 o más)
        for (unsigned int i = primerBL; i <= ultimoBL; i++)
        { //Bloques a leerse, acotados y sucesivos.
            if (i == primerBL)
            { //Primer bloque a leer.
                BF = traducir_bloque_inodo(ninodo, primerBL, 0); //Obtener el bloque de datos a leer por recorrido de punteros de nivel.
                desplazamiento = (offset%BLOCKSIZE); //Margen inicial dentro del bloque. BUGFIX: Tenia que estar fuera del if para que fuera COHERENTE.
                if (BF != -1)
                {
                    bread(BF, buffer); //Leer el contenido previo.
                    //memcpy(buf_original + desplazamiento, buffer, BLOCKSIZE - desplazamiento); //¿Previsible?, lo veo girado.
                    memcpy(buf_original, buffer + desplazamiento, BLOCKSIZE - desplazamiento); //Al leer el bloque, parto desde el desplazamiento del BLOQUE LEIDO.
                }
                bytes_leidos += (BLOCKSIZE - desplazamiento); //Recuento de bytes que fueron leidos desde el margen inicial.
            }
            else if (i == ultimoBL)
            { //Último bloque a leer.
                BF = traducir_bloque_inodo(ninodo, ultimoBL, 0); //Obtener el bloque de datos a leer por recorrido de punteros de nivel.
                desplazamiento2 = nbytes-bytes_leidos; //Posible BUGFIX para incluso con o sin OFFSET. Margen restante para leer (de los que me piden, le resto sobre los que tengo pendientes). BUGFIX: Tenia que estar fuera del if para que fuera COHERENTE.
                if (BF != -1)
                {
                    bread(BF, buffer); //Leer el contenido previo.
                    desplazamiento = offset % BLOCKSIZE;
                    memcpy(buf_original + (BLOCKSIZE - desplazamiento) + (ultimoBL - primerBL - 1) * BLOCKSIZE, buffer, desplazamiento2 + 1); //Estaba girado el filtro OFFSET, tenia que apuntar al buf_original y no al buffer (que es la entrada de 1 bloque, no más).
                }
                bytes_leidos += (desplazamiento2); //Recuento de bytes que fueron leidos. (anteriormente, desplazamiento2+1)
            }
            else
            { //Uno o varios bloques intermedios, estos ocupan siempre bloques enteros.
                BF = traducir_bloque_inodo(ninodo, i, 0); //Obtener el bloque de datos a leer por recorrido de punteros de nivel.
                if(BF >= 0) bread(BF, buf_original + (BLOCKSIZE - desplazamiento) + (i - primerBL - 1) * BLOCKSIZE); //Leer el contenido previo. Falta el BF.
                bytes_leidos += BLOCKSIZE; //Siempre suma el BLOCKSIZE (todo el bloque)
            }
        }
    }
    //Actualizamos el campo del inodo el tiempo de acceso.
    #ifdef ENTREGA_3
    mi_waitSem();
    #endif
    leer_inodo(ninodo, &inodo_lectura); //Recuperar estado del inodo.
    inodo_lectura.atime = time(NULL); //Reinicia el campo de fecha cuando fue accedido.
    escribir_inodo(ninodo, inodo_lectura); //Actualizar el estado del inodo.
    #ifdef ENTREGA_3
    mi_signalSem();
    #endif
    if(INODO_DEV) fprintf(stderr, MSG_VAR("ficheros.c/mi_read_f") "La cantidad de bytes leidos es: %d (+%d offset)\n", bytes_leidos, offset); //Si me lo piden explicitamente, le muestro donde ha tenido lugar.
    return bytes_leidos; //Retorna la cantidad de bytes que fueron leidos del inodo.
}

//Reconvierte la metainformación del STRUCT inodo, en uno de STRUCT stat (omite los punteros).
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    leer_inodo(ninodo, &inodo); //Recuperar el estado del inodo.
    p_stat->tipo = inodo.tipo; //La letra del tipo de inodo replicado { "l: libre", "d: carpeta", "f: fichero" }
    p_stat->permisos = inodo.permisos; //El valor entero del permiso del inodo replicado, { "r: +4", "w: +2", "x: +1" }
    p_stat->atime = inodo.atime; //Replica del acceso a los datos, fecha/hora.
    p_stat->ctime = inodo.ctime; //Replica de creación de los datos, fecha/hora.
    p_stat->mtime = inodo.mtime; //Replica de modificación de los datos, fecha/hora.
    p_stat->nlinks = inodo.nlinks; //Replica de la cantidad de enlaces que apuntan a este inodo.
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados; //Replica de la cantidad de bloques de datos/punteros que tiene en uso.
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog; //Replica de la cantidad de bytes que tiene escritos.
    return 0; //Siempre cierto. El argumento p_stat ya tiene la estructura definida substraida del inodo.
}

//Cambia el marco de permisos del inodo a otro posible.
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    //El arreglo no surte efecto, se me cuela las letras y encima los mismo negativos.
    if(permisos < 48 || permisos > 55) { //Si me piden unos permisos fuera de lugar (!= {r,w,x}); esta en ASCI el valor.
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros.c/mi_chmod_f") "El argumento del permiso no es reconocible. (entre 0 y 7).\n");
        return -1;
    }
    permisos = permisos-48; //Detraer del valor ASCI.
    //Leemos el inodo, actualizamos los permisos y el ctime y lo volvemos a escribir donde coresponde.
    #ifdef ENTREGA_3
    mi_waitSem(); //Inodo, permisos y ctime son criticos
    #endif
    leer_inodo(ninodo, &inodo); //Recuperar el estado del inodo.
    inodo.permisos = permisos; //Actualiza los permisos del inodo.
    inodo.ctime = time(NULL); //Reinicia la fecha de cuando fue creado.
    #ifdef ENTREGA_3
    mi_signalSem();
    #endif
    if(INODO_DEV) fprintf(stderr, MSG_VAR("ficheros.c/mi_chmod_f") "Inodo %d actualizado, el campo de permiso pasa a valer a '%d'.\n", ninodo, inodo.permisos); //Si me lo piden explicitamente, le muestro donde ha tenido lugar.
    escribir_inodo(ninodo, inodo); //Actualizar el estado del inodo.
    return 0; //Siempre cierto.
}

//Elimina todo el contenido escrito posterior a lo indicado, sólo se conservará desde el principio del fichero hasta el final.
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    unsigned int bloques_liberados, numBLogico = 0;

    leer_inodo(ninodo, &inodo); //Recuperar el estado del inodo.
    if ((inodo.permisos & 2) != 2) //Usa un operador binario para ver si contiene el valor del '2' (X1X); si el inodo no tiene permisos de escritura este finaliza con error.
    {
        fprintf(stderr, MSG_ERROR("ficheros.c/mi_truncar_f") "Error truncando el fichero. No tiene permisos de escritura.\n");
        return -1;
    }
    if (inodo.tamEnBytesLog < nbytes)
    { //Si se intenta eliminar datos más allá de lo que tiene escrito previamente, el programa no hace nada.
        if(WARNINGS_LIB) fprintf(stderr, MSG_AVISO("ficheros.c/mi_truncar_f") "Fallo al truncar: no se puede truncar más allá de la longitud del fichero.\n");
        return 0; //Vale 0 lo que contiene (porque no hay nada más allá de lo ecrito).
    }
    //Ninguno lo ha bloqueado, procedemos a retirar los bloques.
    if (nbytes % BLOCKSIZE == 0)
    { //Si es un bloque entero.
        numBLogico = nbytes / BLOCKSIZE;
    }
    else
    { //Si esta fraccionado el bloque, ponderación hacia arriba (siguiente bloque).
        numBLogico = nbytes / BLOCKSIZE + 1;
    }
    bloques_liberados = liberar_bloques_inodo(ninodo, numBLogico); //Libera los bloques ocupados desde el bloque lógico inicial (excluido el bloque inicial, a menos que diga '0' nbytes).
    leer_inodo(ninodo, &inodo); //Recuperar el estado del inodo.
    inodo.tamEnBytesLog = nbytes; //Actualiza la nueva cantidad de bytes escritos.
    inodo.mtime = time(NULL);  //Reinicia el campo de fecha cuando fue modificado.
    inodo.ctime = time(NULL); //Reinicia la fecha de cuando fue creado.
    escribir_inodo(ninodo, inodo); //Actualizar el estado del inodo.

    if(INODO_DEV) fprintf(stderr, MSG_VAR("ficheros.c/mi_truncar_f") "Inodo %d actualizado, tiene %d bytes logicos y %d bloques siguen en uso (%u bloques truncados).\n", ninodo, inodo.tamEnBytesLog, inodo.numBloquesOcupados, bloques_liberados); //Si me lo piden explicitamente, le muestro donde ha tenido lugar.
    return bloques_liberados; //Retorna la cantidad de bloques que fueron suprimidos.
}