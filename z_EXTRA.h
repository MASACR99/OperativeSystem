#include <stdio.h>
#include <stdbool.h>
//Acreditaciones: https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
//Ayuda y tablas: https://en.wikipedia.org/wiki/ANSI_escape_code#Colors

// **   **  GRADO DE VISUALIZACIÓN DE LA INFORMACIÓN.
// ### LIBRERIA
// TIPO: (boolean) 1 -- TRUE; 0 -- FALSE.
//Para ver los bloques que libera, es el BIT_DEV; para el traducir bloques inodos PUNTERO_DEV (por si fuera requerido en la entrega).
#define MODO_DEV 0 //Este ajuste permite inspeccionar con mayor detalle el recorrido de ejecución.
#define RETURN_DEV 0 //Muestra mensajes referentes al return. (bmount, bumount, integridadDisco, initSTRUCT, mi_mkfs)
#define BLOCK_DEV 0 //Permite inspeccionar con mayor detalle los bloques. (bread, bwrite)
#define BIT_DEV 0 //Permite inspeccionar con mayor detalle el comportamiento del mapa de bits. (leer_sf [mostrar todo el mapa], reservar_bloque, liberar_bloque)
#define INODO_DEV 0 //Permite inspeccionar con mayor detalle el comportamiento de los inodos. (escribir_inodo, leer_inodo, reservar_inodo, liberar_inodo, mi_write_f, mi_read_f, mi_chmod_f, mi_truncar_f)
#define PUNTERO_DEV 0 //Permite ver el funcionamiento de los punteros. (traducir_bloque_inodo, liberar_bloques_inodoRECURSIVO [si es recursivo])
#define ENTRADA_DEV 0 //Muestra como ha traducido los nombres a inodos. (extraer_entrada, buscar_entrada, mi_dir)
#define CACHE_DEV 0 //Muestra cuando las caches de directorios.c se accionan.
#define WARNINGS_LIB 0 //Si se quiere mostrar o no los warnings generados en las librerias cuando experimenta un procesamiento anomalo. (*varias funciones*)

//### EJECUTABLE
#define AMPLIAR_EXE 0 //Este ajuste permite inspeccionar con mayor detalle los pasos intermedios (podría llegar a molestar).
#define RETURN_EXE 1 //Establece si los ejecutables han de dar una confirmación o mantener mudo.
#define VERIFICACION_DEV 1 //Establece si ha de mostrar mensajes de los pasos, usarlo si se quiere saber más. [simulacion, verificacion]
#define WARNINGS_EXE 0 //Si se quiere mostrar o no los warnings generados en los ejecutables.

// **   **  OTROS AJUSTES
// TIPO: (boolean) 1 -- TRUE; 0 -- FALSE.
#define LIBERAR_RECURSIVO 1 //Permite alternar entre la versión recursiva (true, más rápido) y la tradicional (false, mejor compatibilidad aunque lento).
#define TRADUCIR_PINODOS 0 //Muestra que inodos estan siendo apuntados por "buscar_entrada".

// **   **  OTROS AJUSTES
// TIPO: (DECLARAR) Sin comentar (#define) -- TRUE; Linea comentada (//#define...) -- FALSE
// #define UNSAFE_CODE habilitar //[SÓLO DESAROLLADORES] Usado para aislar el código no probado, útil para instanciar las versiones (antes y después).
#define ENTREGA_3 habilitar //Habilitar los semáforos mutex/sem de la 3º entrega.
#define REVISAR_UNIDAD habilitar //La función "bmountSAFE" comprueba si la unidad es consistente antes de manipular, esto previene las aperturas de ficheros no reconocibles o esten dañadas (capa de protección inicial).
#define CARACTERISTICA_MMAP habilitar //Permite alojar la unidad del disco a la memoria y acelerar las operaciones. Relevante para: scripts densos, [simulacion, verificacion]

// **   **  COLORES ANSI PREDEFINIDOS
#define COLOR_NEGRO     "\x1b[0m"
#define COLOR_ALERTA    "\x1b[1;4;5;31m"
#define COLOR_ROJO      "\x1b[1;5;31m"
#define COLOR_CHILLON   "\x1b[1;31m"
#define COLOR_VERDE     "\x1b[1;32m"
#define COLOR_AMARILLO  "\x1b[1;33m"
#define COLOR_AZUL      "\x1b[1;34m"
#define COLOR_MAGENTA   "\x1b[1;35m"
#define COLOR_CYAN      "\x1b[1;36m"

// **   **  FORMATO DE MENSAJE EN LA TERMINAL.
//  *** EJEMPLOS -- Se pueden incluir colores en el propio printf si fuese necesario; esto sirve como guia para mostrar el circuito interno.
// IMPORTANTE (FUTURAS IMPLEMENTACIONES): Para el caso de librerias siempre ha de ser "fprintf" usando stderr para preservar los mensajes deseables y no sean exportadas a ficheros externos, esa responsabilidad de incluir o no el mensaje exportado dependerá del ejecutable.
//  printf(MSG_OK("bloques.c/main") "Mensaje el que sea.\n", fichero);        //Muestra la ruta bloques.c/main delimitado por recuadros de color.
//  fprintf(stderr, MSG_ERROR("bloques.c/bwrite") "Mensaje el que sea.\n");       //Este es para el caso de querer mostrar unicamente mensajes de error, puesto que se envia al canal de errores.
#define MSG_OK(ruta) COLOR_VERDE "[V] (" ruta ")" COLOR_NEGRO " -- " //printf, se haga el return válido.
#define MSG_INFO(ruta) COLOR_CYAN "[I] (" ruta ")" COLOR_NEGRO " -- " //printf, notificación sobre un paso.
#define MSG_NOTA(ruta) COLOR_AZUL "[#] (" ruta ")" COLOR_NEGRO " -- " //printf, notificación optativa interna.
#define MSG_VAR(ruta) COLOR_MAGENTA "[@] (" ruta ")" COLOR_NEGRO " -- " //printf, notificación optativa, uso prefebilemente sobre una variable.
#define MSG_AVISO(ruta) COLOR_AMARILLO "[?] (" ruta ")" COLOR_NEGRO " -- " //fprintf, mensajes de aviso en suponer un valor.
#define MSG_ERROR(ruta) COLOR_ROJO "[X] (" ruta ")" COLOR_NEGRO " -- " //fprintf, mensajes obligatorios acerca de los fallos.
#define MSG_FATAL(ruta) COLOR_ALERTA "[!] (" ruta ")" COLOR_NEGRO " -- " //fprintf, mensajes que no debería de aparecer o errores no estipulados.
#define TVAR(precolor,cual) precolor cual COLOR_NEGRO //printf, resalta la cadena 'cual' a un color especifico, el precolor ha de ser una de las definiciones de COLOR ANSI PREDEFINIDOS. Esto es util para resaltar una cadena o una variable a un color provisionalmente y luego reiniciarlo.


// **   **  SELLOS PARA LOS PERMISOS (macro), usar los permisos que tenga de valores [0-7] para mostrarlo por pantalla.
// ** DENTRO DEL CODIGO HAY QUE PONER EN ESTE ORDEN:
// DEFINIR_SELLO_RWX; //Esto hará que se inicialice el array 'SELLO_RWX' para la función actual (como si fuera variable declarada).
// printf("%s como ejemplo, me dara '-w-' \n", SELLO_RWX[2]); //Al usar 'DEFINIR_SELLO_RWX', se ha creado un array llamado 'SELLO_RWX' y con el valor que le pases, obtienes el permiso traducido.
#define DEFINIR_SELLO_RWX char const* SELLO_RWX[8] = ANEXO_ARRAY_SELLO_ITEMS
#define ANEXO_ARRAY_SELLO_ITEMS { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" }