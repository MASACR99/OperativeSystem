# VARIABLES DISPONIBLES
nombreDisco=disco
numbloques=100000
outdirMaked=./../exe
# FUNCIONES PREGENERADAS
function pausarPaso {
    echo
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    echo "Pulse intro para continuar..."
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    echo
    read
}
#Scripts de la entrega del 4/abril/2019 (ejecutar los scripts probados), ejecutar "lanzarTestEntrega1" como comando de bash.
#Los tests sólo se activarían tras probar que funcione todos.
function lanzarTestEntrega1 {
    cp scriptsEntrega/script1e1.sh $outdirMaked
    cp scriptsEntrega/script2e1.sh $outdirMaked
    cp scriptsEntrega/script3e1.sh $outdirMaked
    cp scriptsEntrega/script4e1.sh $outdirMaked
    cd $outdirMaked
    ./script1e1.sh
    pausarPaso
    ./script2e1.sh
    pausarPaso
    ./script3e1.sh
    pausarPaso
    ./script4e1.sh
    echo
    echo "-------------------------------------------------------------"
    echo "-- -- -- -- -- -- -- -- -- $ FIN $ -- -- -- -- -- -- -- -- --"
    exit 0
}
#
# SECUENCIA DE LINEAS A EJECUTAR EN BASH --- AQUI AMPIEZA
#
# El 'make' normal elimina sólo los programas y los *.o; el "clean" borra incluso los ficheros internos.
make clean
make
#VOLCAR EL FICHERO DE EJEMPLO
cp muestraFILE.txt $outdirMaked
#lanzarTestEntrega1 #Para disparar todos los test de prueba (si hubiera)
#Cambiar el directorio
cd $outdirMaked
#
# Por implementacion, siempre ha de empezar mi_mkfs.
#
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo "  mi_mkfs $nombreDisco $numbloques"
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo ""
./mi_mkfs $nombreDisco $numbloques
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
time ./simulacion disco
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
./mi_ls disco /
dir_simulacion=`./mi_ls disco / | tail -n 2 | head -n 1`
sleep 3s
time ./verificacion $nombreDisco /$dir_simulacion/
exit 0
#Si se quiere parar la ejecuccion...
#exit 0
#
# Acceso rapido para las pruebas (atajo)...
echo ""
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo "  _TESTER $nombreDisco amplitudPersonalizada"
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo ""
./mi_mkdir disco 7 /hola/
./mi_mkdir disco 6 /eys/
./mi_touch disco 7 /eys/algo
./mi_touch disco 7 /eys/trasto
./mi_mkdir disco 6 /eys/cosa/
./mi_touch disco 7 /eys/cosa/inquietante
#./mi_mkdir disco 7 /eys/cosa/fails/
./mi_touch disco 7 /eys/genius

./mi_escribir disco /eys/trasto "¿Ola k ase?" 30000

#./mi_chmod disco /hola/ 2

for i in $(seq 0 16)
do
    ./mi_link disco /eys/trasto /hola/$i
done

#./mi_rm disco /hola/

./mi_ls disco / -xi
#./mi_ls disco /hola/ -xi
./mi_ls disco /eys/ -xi
./mi_ls disco /eys/cosa/ -xi

echo 
echo
echo
./mi_cp disco /eys/ /trastero
echo
./mi_ls disco / -xi
#./mi_ls disco /hola/ -xi
./mi_ls disco /eys/ -xi
./mi_ls disco /eys/cosa/ -xi
./mi_ls disco /trastero/ -xi
./mi_ls disco /trastero/cosa/ -xi
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
./mi_cat disco /trastero/trasto



exit 0

./mi_chmod disco "/hola/" 3
./mi_ls disco / -xi
./mi_ls disco /hola/ -xi

exit 0
#PRUEBAS DE LA ENTREGA 2
./mi_mkdir $nombreDisco "2" /hola/
./mi_touch $nombreDisco "5" /hola/ejemplo
#./_TESTER $nombreDisco 0 /hola
#./_TESTER $nombreDisco 0 /ase
./leer_sf $nombreDisco 4
exit 0
#Las pruebas de la etapa 7
#./escribir $nombreDisco Elquesea 0
./escribir $nombreDisco "$(cat muestraFILE.txt)" 0
./escribir $nombreDisco "123456789" 0
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
./leer $nombreDisco 1
./leer $nombreDisco 2
#./truncar $nombreDisco 1 100
exit 0
#Hasta aqui, las pruebas de la 7.
./_TESTER $nombreDisco 8 "$(cat muestraFILE.txt)"
exit 0
./_TESTER $nombreDisco 204
./_TESTER $nombreDisco 30004
./_TESTER $nombreDisco 400004
./_TESTER $nombreDisco 16843019
exit 0
./_TESTER $nombreDisco 0
./_TESTER $nombreDisco 11
./_TESTER $nombreDisco 12
./_TESTER $nombreDisco 13
./_TESTER $nombreDisco 267
./_TESTER $nombreDisco 268
./_TESTER $nombreDisco 269
./_TESTER $nombreDisco 65803
./_TESTER $nombreDisco 65804
./_TESTER $nombreDisco 65805
./_TESTER $nombreDisco 6400000
#Si se quiere parar la ejecuccion...
exit 0
#
echo ""
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo "  leer_sf $nombreDisco"
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
echo ""
./leer_sf $nombreDisco