### NOMBRES DEL GRUPO:
Joan, Gil Rigo
Christian, Ortega Ordinas
Eloy, Tolosa Gómez

### COMPOSICION INTERNA:
· README.txt (este documento que usted esta leyendo)
· "src" (carpeta)
··· bloques.c/h
··· escribir.c
··· ficheros_basico.c/h
··· ficheros.c/h
··· leer_sf.c/h
··· leer.c
··· Makefile
··· mi_mkfs.c/h
··· permitir.c
··· truncar.c
··· z_EXTRA.h

### COMPILAR Y CONSTRUIR:
· "make" Rehacer los programas ejecutables. Cada vez que se haga un cambio (incluso en z_extra) y quiera conservar los ficheros contenidos en la carpeta "exe".

· "make demiclean" Elimina los programas ejecutables. Si se quiere desinstalar o suprimir el programa y quiera CONSERVAR los ficheros contenidos en la carpeta "exe".

· "make clean" Elimina TODO EL CONTENIDO de la carpeta "exe", no nos hacemos responsables del contenido perdido y se haya olvidado respaldar dentro del "exe", resulta útil si no quiere conservar los ficheros "dispositivo" generados.

### SINTAXIS DE LOS PROGRAMAS:
LEYENDA: <argumento>, valor requerido para funcionar. ;;; #argumento#, valor OPCIONAL (usar un predeterminado si se omite)
· mi_mkfs <nombre_fichero> <capacidad_de_bloques>   #Permite crear un fichero X como dispositivo, asignandole X capacidad de bloques.
· leer_sf <nombre_fichero> #grado_muestreo#     [(ausencia del "#grado_muestreo#") sólo superbloque, '0' TODOS los campos, '1' sólo mapa de bits, '2' sólo array de inodos, '3' sólo el sizeOf estructural, '4' sólo los inodos ocupados.]
· escribir <dispositivo> <texto> <diferentes inodos>    #Permite escribir un texto a un único inodo (diferentes_inodos = 0), o bien en varios inodos (diferentes_inodos != 0)
· leer <dispositivo> <número de inodo>      #Leer el contenido de un inodo especifico.
· truncar <nombre_dispositivo> <ninodo> <nbytes>    #Elimina desde la posición X hacia ADELANTE, todo el contenido escrito. Sólo conservará el contenido hasta llegar los X bytes.
· permitir <nombre_dispositivo> <ninodo> <permisos>     #Cambia la regulación de los permisos del inodo. Los permisos es un valor entero entre [0-7].

### EJEMPLO DE USO DE LOS PROGRAMAS:
· mi_mkfs disco 100000
· leer_sf disco
· leer_sf disco 0
· escribir disco "hola mundo" 0
· escribir disco "$(cat ejemplo.txt)" 0
· escribir disco "hola mundo" 989888
· escribir disco "$(cat ejemplo.txt)" 1
· leer disco 1
· truncar disco 1 100
· permitir disco 1 3

### RESTRICCIONES DEL PROGRAMA:
· El programa "mi_mkfs" es el ÚNICO quien puede crear el fichero "dispositivo", ningún otro ejecutable puede funcionar sin haber operado el "mi_mkfs" previamente.
· "mi_mkfs" necesita que la capacidad_de_bloques sea de "4" o superior.
· "escribir" necesita que tenga algún texto introduccido, si se omite, el programa aborta.
· "leer" espera que tenga algún texto introduccido en el inodo, si no tiene, el programa aborta.

### PROBLEMAS CONOCIDOS:
· Cualquier ejecutable puede montar un fichero existente aunque no sea el "dispositivo", ahora bien, internamente occure todo tipo de errores y existe un riesgo de dañar el fichero utilizado (esta confirmado con los programas "permitir, escribir"). EVITE usar ficheros ajenos que no sean los generados por mi_mkfs.
· Truncar podría mostrar datos anormales del inodo si ese inodo nunca fue reservado/usado previamente.

### MEJORAS CONSIDERABLES:
· Makefile maneja un directorio de trabajo, genera una carpeta para los programas a la ruta designada donde se compila y puede borrarse.
· Mensajes con formato, los mensajes de la terminal vienen con color y estructurados para agilizar la lectura.
· Regulación de errores, ponemos freno a los problemas comunes que pudieran dar lugar.
· Código substancialmente comentado, no nos hemos quedado cortos en explicar que hace.
· Ocultación y visualización de los mensajes personalizable, la cabecera de "z_EXTRA.h" permite delimitar o mostrar más información sobre lo que ocurre dentro.

