#ifndef TABLAS_ADMINISTRATIVAS_H_
#define TABLAS_ADMINISTRATIVAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
	int espacioMaximo;
	bool disponible;
	tiempo ultimaModificacion;
	int primerLetra;
	int ultimaLetra;
	char * trabajoActual;
	t_list* claves;
}t_instancia;

typedef struct{
	char* nombre;
	int socket;
}t_conexion;


sem_t g_mutex_tablas;

/*-------------------FUNCIONES---------------------------*/

t_instancia* traerInstanciaMasEspacioDisponible(t_list* tablaDeInstancias);
t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias);
t_instancia* traerInstanciaQueContieneKey(t_list* tablaDeInstancia,char* primerLetraClave);
void distribuirKeys(t_list* tablaDeInstancias);

t_list* crearListaInstancias(void);
t_instancia* crearInstancia(char* nombre, int espacioMaximo);
void agregarInstancia(t_list * lista, t_instancia* instancia );
void destruirInstancia(t_instancia * instancia);
void mostrarInstancia(t_instancia * instancia);
void mostrarEspacioOcupado(t_instancia* instancia);
void mostrarTablaInstancia(t_list* tablaDeInstancias);
t_instancia* buscarInstancia(t_list* tablaDeInstancias,bool buscaInstanciasNoDisponibles ,char* nombre,int letraAEncontrar, char* clave);
void eliminiarClaveDeInstancia(t_list* claves, char* claveAEliminar);
bool instanciaContieneClave(t_list* claves,char* clave);

t_list* crearDiccionarioConexiones(void);
t_conexion* crearConexion(char* nombre, int socket);
void agregarConexion(t_list * diccionario, char * clave , int valor);
void sacarConexion(t_list* diccionario, t_conexion* conexion);
void mostrarDiccionario(t_list* diccionario);
t_conexion* buscarConexion(t_list * diccionario, char * clave, int socket);
void cerrarConexion(void* conexion);
void destruirConexion(void* conexion);
void cerrarTodasLasConexiones(t_list* diccionario);
void destruirDiccionario(t_list* diccionario);

#endif /* TABLAS_ADMINISTRATIVAS_H_ */
