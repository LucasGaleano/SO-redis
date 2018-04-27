#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <biblioteca/sockets.h>
#include <biblioteca/paquetes.h>

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/coordinador.cfg"


/*---------------------Estructuras-------------------------*/

typedef struct t_configuracion{
	int puertoConexion;
	char* algoritmoDist;
	int cantidadEntradas;
	int tamanioEntradas;
	int retardo;
}t_configuraciones;


/*------------------------Globales-------------------------*/

t_configuraciones g_configuracion;


/*------------------------FUNCIONES-------------------------*/

t_configuraciones armarConfigCoordinador(t_config*);


#endif /* COORDINADOR_H_ */
