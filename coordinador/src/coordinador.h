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

#define PATH_CONFIG "/home/lucas/workspace/tp-2018-1C--0/configuraciones/coordinador.cfg"


/*---------------------Estructuras-------------------------*/

typedef struct t_configuracion{
	int puertoConexion;
	char* algoritmoDist;
	int cantidadEntradas;
	int tamanioEntradas;
	int retardo;
}t_configuraciones;


/*------------------------Globales-------------------------*/

t_log* g_logger;
t_configuraciones g_configuracion;
t_list* g_tablaDeInstancias;
t_dictionary* g_diccionarioConexiones;
sem_t g_mutexLog;


/*------------------------FUNCIONES-------------------------*/
void procesarHandshake(t_paquete* paquete,int socketCliente);
void procesarSET(t_claveValor* sentencia, int socketCliente);
void procesarGET(char* clave,int socketCliente);
void procesarSTORE(char* clave,int socketCliente);
void procesarNombreESI(char* nombreESI, int socketCliente);
void procesarNombreInstancia(char* nombre, int socketCliente);
void procesarRespuestaSET(int respuesta,int socketCliente);
void logearRespuesta(int respuesta, t_instancia* instancia);
void logTraceSeguro(t_log* logger,sem_t a,char* format,...);
t_configuraciones armarConfigCoordinador(t_config*);
t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion,
																	char* Clave,
	 																t_list* tablaDeInstancias);

#endif /* COORDINADOR_H_ */
