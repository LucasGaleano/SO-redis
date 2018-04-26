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
#include <commons/collections/dictionary.h>
#include "planificador.h"



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
char* 				obtenerParametro						(char*, int);
bool 				estaBloqueado							(char* clave);
bool 				estaEjecutando							(char* clave);
bool 				estaBloqueadoPorUsuario					(char* clave);
bool 				estaBloqueadoPorElRecurso				(char* claveESI, char* claveRecurso);

#endif /* CONSOLA_H_ */
