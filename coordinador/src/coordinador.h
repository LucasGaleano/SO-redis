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
#include <signal.h>

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/coordinador.cfg"


/*---------------------Estructuras-------------------------*/

typedef struct{
	char* puertoConexion;
	char* algoritmoDist;
	int cantidadEntradas;
	int tamanioEntradas;
	int retardo;
}t_configuraciones;


/*------------------------Globales-------------------------*/

t_log* g_logger;
t_configuraciones g_configuracion;
t_list* g_tablaDeInstancias;
t_list* g_diccionarioConexiones;
sem_t g_mutexLog;



/*------------------------FUNCIONES-------------------------*/
void planificador_handler(int signum);
int iniciarServidor(char* puerto);
void* procesarPeticion(int* cliente_fd);
void 	procesarPaquete(t_paquete* unPaquete,int socketCliente);
void procesarHandshake(t_paquete* unPaquete,int socketCliente);
void procesarSET(t_paquete* unPaquete,int socketCliente);
void procesarGET(t_paquete* unPaquete,int socketCliente);
void procesarSTORE(t_paquete* unPaquete,int socketCliente);
void procesarNombreESI(t_paquete* unPaquete,int socketCliente);
void procesarNombreInstancia(t_paquete* unPaquete,int socketCliente);
void procesarRespuesta(t_paquete* unPaquete,int socketCliente);
void procesarClienteDesconectado(int cliente_fd);
void logTraceSeguro(t_log* logger,sem_t a,char* format,...);
t_configuraciones armarConfigCoordinador(t_config*);
t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion,
																	char* Clave,
	 																t_list* tablaDeInstancias);

#endif /* COORDINADOR_H_ */
