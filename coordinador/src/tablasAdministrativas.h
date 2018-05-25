#ifndef TABLAS_ADMINISTRATIVAS_H_
#define TABLAS_ADMINISTRATIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <time.h>

/*TABLA DE INSTANCIAS
 *
 *		{nombre de instancia, espacio ocupado(entradas), 	rango, 				 disponible, 	socket	     ultimaModificada			trabajoActual}
 *		 instancia1,          50 													97-106(a-j)    true   		  1  				18:12:1234							SET:k1045
 *		 instancia2			    	32						 							107-112(k-o)   true	      	2	 				18:12:4312							GET:k3042
 */

/*-------------------ESTRUCTURAS---------------------------*/

typedef struct{
	char* nombre;
	int espacioOcupado;
	bool disponible;
	int socket;
	time_t ultimaModificacion;
	int primerLetra;
	int ultimaLetra;
	char * trabajoActual;
}t_instancia;

t_dictionary *  g_ESI;

/*-------------------FUNCIONES---------------------------*/

t_instancia* traerInstanciaMasEspacioDisponible(t_list* tablaDeInstancias);
t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias);
void distribuirKeys(t_list* tablaDeInstancias);

t_list* crearListaInstancias(void);
void agregarInstancia(t_list * lista, t_instancia* instancia );
t_instancia* crearInstancia(char* nombre,int socket);
void destruirInstancia(t_instancia * instancia);
void mostrarInstancia(t_instancia * instancia);
t_instancia* buscarInstancia(t_list* tablaDeInstancias, char* nombre,int primerLetra,int socket);

t_dictionary* crearDiccionarioESI(void);
void agregarEsi(t_dictionary * diccionario, char * nombre , int valor);


#endif /* TABLAS_ADMINISTRATIVAS_H_ */
