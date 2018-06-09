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
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/coordinador.cfg"


/*---------------------Estructuras-------------------------*/

typedef struct t_configuracion{
	int puertoConexion;
	char* algoritmoDist;
	int cantidadEntradas;
	int tamanioEntradas;
	int retardo;
}t_configuraciones;


typedef struct {
	t_paquete* paquete;
	int socket;
}pthreadArgs_t;


/*------------------------Globales-------------------------*/

t_log* g_logger;
t_configuraciones g_configuracion;
t_list* g_tablaDeInstancias;
t_dictionary* g_diccionarioConexiones;
sem_t g_mutexLog;



/*------------------------FUNCIONES-------------------------*/
void* procesarHandshake(pthreadArgs_t* args);
void* procesarSET(pthreadArgs_t* args);
void* procesarGET(pthreadArgs_t* args);
void* procesarSTORE(pthreadArgs_t* args);
void* procesarNombreESI(pthreadArgs_t* args);
void* procesarNombreInstancia(pthreadArgs_t* args);
void* procesarRespuesta(pthreadArgs_t* args);
void logTraceSeguro(t_log* logger,sem_t a,char* format,...);
t_configuraciones armarConfigCoordinador(t_config*);
t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion,
																	char* Clave,
	 																t_list* tablaDeInstancias);

#endif /* COORDINADOR_H_ */
