CC=gcc
CFLAGS=-c -g -Wall -std=gnu99
LDFLAGS=-pthread
#Ruta de salida predeterminada para que funcione con el estandar IDE.
OUTDIR=./../exe/
#En el caso de volcar a una maquina virtual Unix (en red) y configuracion basica, apuntar a esta posicion la salida.
#OUTDIR=~/Escritorio/exe/

SOURCES=bloques.c ficheros_basico.c ficheros.c directorios.c mi_mkfs.c leer_sf.c mi_chmod.c mi_mkdir.c mi_touch.c mi_ls.c mi_escribir.c mi_cat.c mi_stat.c mi_link.c mi_unlink.c mi_mv.c mi_rm.c mi_rmdir.c mi_rmdir_r.c mi_cp.c mi_cp_f.c semaforo_mutex_posix.c verificacion.c simulacion.c
LIBRARIES=bloques.o ficheros_basico.o ficheros.o directorios.o semaforo_mutex_posix.o
INCLUDES=bloques.h ficheros_basico.h ficheros.h directorios.h mi_mkfs.h leer_sf.h z_EXTRA.h semaforo_mutex_posix.h verificacion.h simulacion.h
PROGRAMS=mi_mkfs leer_sf mi_chmod mi_mkdir mi_touch mi_ls mi_escribir mi_cat mi_stat mi_link mi_unlink mi_mv mi_rm mi_rmdir mi_rmdir_r mi_cp mi_cp_f simulacion verificacion

#objs 'los que compila', librariesobjs 'agrega el prefijo OUTdir en cada entrada de libraries' (compilar 2º fase), programsclear 'idem de prefijos en cada programa' (demiclear)
OBJS=$(SOURCES:.c=.o)
LIBRARIESOBJS=$(patsubst %,$(OUTDIR)%,$(LIBRARIES))
PROGRAMSCLEAR=$(patsubst %,$(OUTDIR)%,$(PROGRAMS))


# !!! ORDEN DE ARRANQUE AL COMPILAR (cuando hay ausencia de argumentos en el make)
all: inici demiclean $(OBJS) $(PROGRAMS) compclear

# !! FUNCIONES DISPONIBLES (cuando se pasa con el argumento, o son definidas en el orden de arranque)
#Genera la carpeta donde se va a exportar los ficheros compilados, '-p' => ignora el aviso en caso de que ya exista.
inici:
	mkdir -p $(OUTDIR)

#Suprime los objetos *.o (ficheros huerfanos) que fueron usados durante la compilacion, usar después de compilar.
compclear:
	rm -rf $(OUTDIR)*.o
#	rm -rf *.o *~ $(PROGRAMS)

#Suprime los objetos *.o (ficheros huerfanos) y los programas generados, esto puede ser util si se pretende conservar los ficheros contenidos.
demiclean:
	rm -rf $(OUTDIR)*.o *~ $(PROGRAMSCLEAR)

#Suprime TODOS los elementos de la carpeta donde se exporta (OUTDIR), usar antes de iniciar la compilación.
#Esta puesto todos porque no se sabe el nombre de los futuros ficheros.
progclear:
	rm -rf $(OUTDIR)*


#2 etapa (compilar), las que estan marcadas como librerias se combina con los programs "gcc 1.o 2.o p.o -o p"
$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $(LDFLAGS) $(LIBRARIESOBJS) $(OUTDIR)$@.o -o $(OUTDIR)$@
#	$(CC) $(LDFLAGS) $(LIBRARIES) $@.o -o $(OUTDIR)$@

#1 etapa (compilar), todos los *.c son compilados *.o "ggc ... -o *.o -c *.c"
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $(OUTDIR)$@ -c $<
#	$(CC) $(CFLAGS) -o $@ -c $<

#Para suprimir el programa y los ficheros contenidos (es una carpeta para exportar los ejecutables, no para guardar) 
.PHONY: clean
clean: progclear
