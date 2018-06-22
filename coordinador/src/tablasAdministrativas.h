#ifndef TABLAS_ADMINISTRATIVAS_H_
#define TABLAS_ADMINISTRATIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/log.h>
#include <semaphore.h>

/*TABLA DE INSTANCIAS
 *
 *		{nombre de instancia, espacio ocupado(entradas), 	rango, 				 disponible, 	     ultimaModificada			trabajoActual   claves}
 *		 instancia1,          50 													97-106(a-j)    true     				18:12:1234							SET:k1045
 *		 instancia2			    	32						 							107-112(k-o)   true	  	 				18:12:4312							GET:k3042
 */

/*-------------------ESTRUCTURAS---------------------------*/
typedef int tiempo;
tiempo g_tiempoPorEjecucion;

typedef struct{
	char* nombre;
	int espacioOcupado;
	bool disponible;
	tiempo ultimaModificacion;
	int primerLetra;
	int ultimaLetra;
	char * trabajoActual;
	t_list* claves;
}t_instancia;

sem_t g_mutex_tablas;

/*-------------------FUNCIONES---------------------------*/

t_instancia* traerInstanciaMasEspacioDisponible(t_list* tablaDeInstancias);
t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias);
t_instancia* traerInstanciaQueContieneKey(t_list* tablaDeInstancia,char* primerLetraClave);
void distribuirKeys(t_list* tablaDeInstancias);

t_list* crearListaInstancias(void);
void agregarInstancia(t_list * lista, t_instancia* instancia );
t_instancia* crearInstancia(char* nombre);
void destruirInstancia(t_instancia * instancia);
void mostrarInstancia(t_instancia * instancia);
void mostrarTablaInstancia(t_list* tablaDeInstancias);
t_instancia* buscarInstancia(t_list* tablaDeInstancias,bool buscaInstanciasNoDisponibles ,char* nombre,int letraAEncontrar);
void eliminiarClaveDeInstancia(t_list* claves, char* claveAEliminar);

t_dictionary* crearDiccionarioConexiones(void);
char* buscarDiccionarioPorValor(t_dictionary* diccionario, int* valor);
void mostrarDiccionario(t_dictionary* diccionario);
void agregarConexion(t_dictionary * diccionario, char * clave , int* valor);
int* conseguirConexion(t_dictionary * diccionario, char * clave);
void cerrarTodasLasConexiones(t_dictionary * diccionario);


#endif /* TABLAS_ADMINISTRATIVAS_H_ */
