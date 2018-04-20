/*
 * consola.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "planificador.h" // ?? no se si va, estaba pensando que se podria usar para pausarPlanificación

/*------------------------------Consola------------------------------*/
void 				iniciarConsola							(void);
void 				ejecutarComando							(char *, bool *);

/*------------------------------Comandos------------------------------*/
void 				pausarPlanificacion						(void);
void				salirConsola							(bool* ejecutar);
void 				continuarPlanificacion					(void);
void 				bloquearESI								(char* linea);
void 				desbloquearESI							(char* linea);

/*------------------------------Auxiliares------------------------------*/
char* 				obtenerParametro						(char *, int);

#endif /* CONSOLA_H_ */
