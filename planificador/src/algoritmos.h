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
#include <biblioteca/estructuras.h>
#include <commons/collections/list.h>

#include "globales.h"

int g_termino;
int g_bloqueo;
int g_tEjecucion;
int g_claveTomada;

int g_socketEnEjecucion;
int g_huboModificacion;
char* g_claveGET;
char* g_idESIactual;

void planificarSinDesalojo(char*);
char* calcularSiguiente(double (*calculadorProx) (double, double, double), int(*ponderacion)(int, int));
void bloquear(t_infoListos * bloq, int nuevoReal, char* key);
char* asignarID(int val, char* ret);
int esMenor(int comp1, int comp2);
int esMayor(int comp1, int comp2);


#endif /* ALGORITMOS_H_ */
