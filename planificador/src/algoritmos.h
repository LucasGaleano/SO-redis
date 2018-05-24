/*
 * algoritmos.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef ALGORITMOS_H_
#define ALGORITMOS_H_

#include <time.h>
#include <commons/string.h>
#include <stdlib.h>

#include "globales.h"

int g_termino;
int g_bloqueo;
int g_tEjecucion;

int g_socketEnEjecucion;
int g_huboModificacion;
char* g_claveGET;

void planificarSinDesalojo(char*);
void gestionarRespuestaESI(t_paquete* unPaquete, int* socket);
void gestionarRespuestaCoordinador(t_paquete* unPaquete, int* socket);
double calcularProximaRafaga(double estimadoAnterior, double realAnterior);
void bloquear(t_infoListos * bloq, int nuevoReal, char* key);
char* asignarID(int val, char* ret);


#endif /* ALGORITMOS_H_ */
