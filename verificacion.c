#include "verificacion.h"

int main(int argc, char **args)
{
    struct STAT p_stat;
    struct entrada buff_lect[NUMPROCESOS * sizeof(struct entrada)];
    struct INFORMACION info;
    int cant_registros_buffer_escrituras = 256, ninodo = 0;
    struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
    struct REGISTRO eof;
    unsigned int offset = 0, p_inodo = 0, p_entrada = 0, p_inodo_dir = 0;
    char dir[60], dir2[60], str_info[BLOCKSIZE], str_aux[BLOCKSIZE];
    memset(str_info, 0, BLOCKSIZE);
    memset(str_aux, 0, BLOCKSIZE);
    memset(dir2, 0, strlen(dir2));
    memset(dir, 0, strlen(dir));

    if (argc < 2)
    {
        fprintf(stderr, MSG_ERROR("verificacion.c") "La correcta llamada a simulacion deberia ser: './verificacion <disco> <directorio_simulacion>' \n");
        return -1;
    }
    bmountSAFE(args[1]);
    mi_stat(args[2], &p_stat, &ninodo);
    if ((p_stat.tamEnBytesLog / sizeof(struct entrada)) != NUMPROCESOS)
    {
        fprintf(stderr, MSG_ERROR("verificacion.c") "Nentradas != NUMPROCESOS (¿habias lanzado el verificar previamente?)\n");
        return -1;
    }
    //Crear informe.txt en directorio de simulacion
    info.nEscrituras = 0; //inicializar nEscrituras a 0
    strcpy(dir, args[2]);
    strcat(dir, "informe.txt"); //La variable 'dir' ya es un directorio, no hay que ponerle / al principio
    mi_creat(dir, '7', '1');
    printf(MSG_OK("verifficacion.c") "Creado fichero %s con exito.\n", dir);
    if(commonAPI_pinodo(args[2], &p_inodo_dir, &p_inodo, &p_entrada, '0', '7', '2') < 0) return -1;
    //printf(MSG_VAR("verificacion.c") "p_inodo_dir: %d, p_inodo: %d, p_entrada: %d.\n", p_inodo_dir, p_inodo, p_entrada); //Lo podemos descartar el pInodoDir.
    mi_read_f(p_inodo_dir, buff_lect, 0, NUMPROCESOS * sizeof(struct entrada)); //Mejora, leemos al buffer 1 vez en lugar de cada iteración del for
    for (int i = 0; i < NUMPROCESOS; i++)
    {
        //Leer entrada y extraer PID, guardar en registro info
        info.pid = atoi(strchr(buff_lect[i].nombre, '_')+1); //Pasamos lo leido a int para tener el pid
        if(VERIFICACION_DEV) printf(MSG_VAR("verificacion.c") "Pid del proceso a validar: %d.\n", info.pid);
        strcpy(dir2,args[2]);
        strcat(dir2, buff_lect[i].nombre);
        strcat(dir2, "/pruebadat"); //BUGFALL: /prueba.dat provoca corrupcion de memoria en buscar_entrada.
        if(VERIFICACION_DEV) printf(MSG_VAR("verificacion.c") "Directorio de prueba: %s.\n", dir2);
        offset = 0;
        memset(buffer_escrituras, 0, cant_registros_buffer_escrituras * sizeof(struct REGISTRO));
        while (mi_read(dir2, buffer_escrituras, offset, cant_registros_buffer_escrituras * sizeof(struct REGISTRO)) > 0)
        {
            for (unsigned int j = 0; (j < cant_registros_buffer_escrituras); j++)
            {
                //haya escrituras en prueba.dat
                if (buffer_escrituras[j].pid == info.pid)
                {                              //escritura válida
                    if (info.nEscrituras == 0) // Esta es la primera escritura validada
                    {
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                    }
                    else
                    {
                        if (difftime(buffer_escrituras[j].fecha, info.PrimeraEscritura.fecha) < 0) // Significa que la primera en escribirse es buffer_escrituras[i]
                        {
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }
                        else if (difftime(buffer_escrituras[j].fecha, info.UltimaEscritura.fecha) > 0) // Significa que la ultima escritura fue buffer_escrituras[i]
                        {
                            info.UltimaEscritura = buffer_escrituras[j];
                        }
                        else //Si las fechas son las mismas, hay que mirar el número de escritura
                        {
                            if (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura) // Significa que la primera escritura fue buffer_escrituras[i]
                            {
                                info.PrimeraEscritura = buffer_escrituras[j];
                            }
                            else if (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura) // Significa que la última escritura fue buffer_escrituras[i]
                            {
                                info.UltimaEscritura = buffer_escrituras[j];
                            }
                        }
                    }
                    info.nEscrituras++;
                }
            }
            memset(buffer_escrituras, 0, cant_registros_buffer_escrituras * sizeof(struct REGISTRO)); //limpiamos buffer
            offset += cant_registros_buffer_escrituras * sizeof(struct REGISTRO);
        }
        //obtener escritura ultima posicion, delimitada por EOF
        mi_stat(dir2, &p_stat, &ninodo);
        mi_read(dir2, &eof, p_stat.tamEnBytesLog - sizeof(struct REGISTRO), sizeof(struct REGISTRO));
        info.MayorPosicion = eof;
        //añadir informacion del struct info a informe.txt(escribir info.Mayor, Menor, nEescrituras, Pid, primer y ultimo)
        if(VERIFICACION_DEV) fprintf(stderr, MSG_OK("verificacion.c") "%d) %d escrituras validadas en %s.\n", i+1, info.nEscrituras, dir2);
        strcat(str_info, "PID: ");
        sprintf(str_aux, "%d\n", info.pid);
        strcat(str_info, str_aux);
        strcat(str_info, "Numero escrituras: ");
        sprintf(str_aux, "%d\n", info.nEscrituras);
        strcat(str_info, str_aux);
        sprintf(str_aux, "Primera escritura:  %i  %i  %s\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)));
        strcat(str_info, str_aux);
        sprintf(str_aux, "Ultima escritura:  %i  %i  %s\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)));
        strcat(str_info, str_aux);
        sprintf(str_aux, "Mayor posicion:  %i  %i  %s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));
        strcat(str_info, str_aux);
        sprintf(str_aux, "Menor posicion:  %i  %i  %s\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)));
        strcat(str_info, str_aux);

        mi_write(dir, str_info, (NUMPROCESOS - i - 1) * BLOCKSIZE, BLOCKSIZE);

        memset(str_info, 0, BLOCKSIZE);
        memset(str_aux, 0, BLOCKSIZE);
        memset(dir2, 0, strlen(dir2));

        info.nEscrituras = 0;
    }
    bumount();
    exit(EXIT_SUCCESS);
}