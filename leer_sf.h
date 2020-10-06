#include <stdio.h> 
#include <stdlib.h> 


int main(int argc, char **argv);
int separarFunciones(); //Agregar una separacion visible, como un delimitador.
int mostrarSuperbloque(); //Muestra como esta compuesto el superbloque.
int mostrarMapabits(); //Muestra la asignacion de los bloques (libre/ocupado), puede funcionar como modo resumido y modo completo.
int mostrarArrayinodo(); //Muestra los bloques donde se ubican los inodos, es posible que pueda mostrar los detalles de ciertos inodos.
int mostrarSizeofs(); //Muestra otra información que puede servir si esta bien definida la estructura.
int mostrarInodos(); //Muestra los inodos creados. "PENDIENTE DE IMPLEMENTACION"