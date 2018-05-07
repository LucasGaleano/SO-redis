#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include "tablasAdministrativas.h"
#include <commons/config.h>
#include <commons/log.h>
#include <biblioteca/sockets.h>
#include <biblioteca/paquetes.h>
#include <biblioteca/estructuras.h>

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
t_list* g_tablaDeInstancias;


/*------------------------FUNCIONES-------------------------*/

t_configuraciones armarConfigCoordinador(t_config*);


#endif /* COORDINADOR_H_ */
