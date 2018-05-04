#ifndef TABLAS_ADMINISTRATIVAS_H_
#define TABLAS_ADMINISTRATIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>

/*TABLA DE INSTANCIAS
 *
 *		{nombre de instancia, espacio ocupado(entradas), rango, 		disponible,   ip:puerto,  	     ultimaModificada}
 *	EJ:	 instancia1,          50 						 97-106(a-j)    true   		  127.012.12.2:4444  18:12:1234
 *		 instancia2			  32						 107-112(k-o)   true	      168.127.0.1:3333	 18:12:4312
 */

typedef struct t_instancia {
	char* nombre;
	int espacio;
	bool disponible;
	char* ipPuerto;
	time_t ultimaModificacion;
	int primerLetra;
	int ultimaLetra;
}instancia;

t_list * crearListaInstancias();

void agregarInstancia(t_list * lista, instancia* instancia );

instancia * crearInstancia(char* nombre,int espacio,char* ipPuerto,
							time_t ultimaModificacion,int primerLetra,int ultimaLetra);

void destruirInstancia(instancia * instancia);

void mostrarInstancia(instancia * instancia);


#endif /* TABLAS_ADMINISTRATIVAS_H_ */
