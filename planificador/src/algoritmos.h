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

#include "planificador.h"

int g_termino;
int g_bloqueo;
int g_finalizo;
int g_tEjecucion;

void planificarSinDesalojo(char*);
void gestionarRespuestaESI(t_paquete* unPaquete, int* socket);


#endif /* ALGORITMOS_H_ */
