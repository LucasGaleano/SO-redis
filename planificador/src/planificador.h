#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <biblioteca/sockets.h>
#include <biblioteca/paquetes.h>
#include <biblioteca/estructuras.h>

#define RUTA_CONFIGURACION_PLANIF "home/utnso/workspace/configuraciones/planificador.cfg"

typedef struct {
	int socketESI, tEnEspera;
	double estAnterior, realAnterior;
} t_infoListos;

typedef struct {
	char* idESI;
	t_infoListos* data;
}t_infoBloqueo;

pthread_mutex_t mutexConsola = PTHREAD_MUTEX_INITIALIZER;

t_dictionary* g_listos;
t_dictionary* g_bloq;

t_log* g_logger;
t_config* g_con;

double g_est;
int g_socketCoordinador;

void procesarPaquete(t_paquete*, int*);
void recibirHanshakePlanif(t_paquete*, int*);
void iniciarServidor(int);
void asignarBloquedas(char**);

#endif /* PLANIFICADOR_H_ */
