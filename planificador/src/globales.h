/*
 * globales.h
 *
 *  Created on: 24 may. 2018
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <biblioteca/sockets.h>
#include <biblioteca/paquetes.h>
#include <biblioteca/estructuras.h>

typedef struct {
	int socketESI;
	int tEnEspera;
	double estAnterior;
	double realAnterior;
} t_infoListos;

typedef struct {
	char* idESI;
	t_infoListos* data;
}t_infoBloqueo;

typedef struct {
 	char* clave;
 }t_infoClavesBloqueadas;

static pthread_mutex_t mutexConsola = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutexBloqueo = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutexListo = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t modificacion = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ESIentrada = PTHREAD_COND_INITIALIZER;

t_dictionary* g_listos;
t_dictionary* g_bloq;
t_dictionary* g_clavesBloqueadas;

t_log* g_logger;
t_config* g_con;

double g_est;
int g_socketCoordinador;
char* g_algoritmo;
double g_alfa;

#endif /* GLOBALES_H_ */
