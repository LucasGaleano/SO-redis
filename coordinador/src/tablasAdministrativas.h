#ifndef TABLAS_ADMINISTRATIVAS_H_
#define TABLAS_ADMINISTRATIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <time.h>

/*TABLA DE INSTANCIAS
 *
 *		{nombre de instancia, espacio ocupado(entradas), 	rango, 				 disponible, 	socket	     ultimaModificada}
 *		 instancia1,          50 													97-106(a-j)    true   		  1  				18:12:1234
 *		 instancia2			    	32						 							107-112(k-o)   true	      	2	 				18:12:4312
 */

/*-------------------ESTRUCTURAS---------------------------*/

typedef struct{
	char* nombre;
	int espacioOcupado;
	bool disponible;
	int* socket;
	time_t ultimaModificacion;
	int primerLetra;
	int ultimaLetra;
}t_instancia;

/*-------------------FUNCIONES---------------------------*/

t_instancia*	 traerInstanciaMasEspacioDisponible 		  (t_list* tablaDeInstancias);
t_instancia*	 traerUltimaInstanciaUsada				(t_list* tablaDeInstancias);
t_list *       crearListaInstancias				(void);
void           agregarInstancia           (t_list * lista, t_instancia* instancia );
t_instancia*   crearInstancia        		(char* nombre,int espacio,int* socket,
							                            	time_t ultimaModificacion,int primerLetra,int ultimaLetra);
void           destruirInstancia          (t_instancia * instancia);
void           mostrarInstancia           (t_instancia * instancia);


#endif /* TABLAS_ADMINISTRATIVAS_H_ */
