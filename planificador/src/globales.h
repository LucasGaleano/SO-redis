/*
 * globales.h
 *
 *  Created on: 24 may. 2018
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <commons/log.h>
#include <semaphore.h>

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

pthread_mutex_t mutexConsola;
pthread_mutex_t mutexBloqueo;
pthread_mutex_t mutexListo;
pthread_mutex_t modificacion;
pthread_mutex_t mutexLog;
sem_t ESIentrada;
sem_t continua;

t_dictionary* g_listos;
t_dictionary* g_bloq;
t_dictionary* g_clavesTomadas;

t_log* g_logger;
t_config* g_con;

double g_est;
int g_socketCoordinador;
char* g_algoritmo;
double g_alfa;

#endif /* GLOBALES_H_ */
