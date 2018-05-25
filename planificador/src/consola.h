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
#include <pthread.h>
#include "globales.h"

/*------------------------------Globales-----------------------------*/
char* g_idComparar;

/*------------------------------Consola------------------------------*/
void 				iniciarConsola							(void);
void 				ejecutarComando							(char *, bool *);

/*------------------------------Comandos------------------------------*/
void 				ejecutarMan								(void);
void 				pausarPlanificacion						(void);
void 				continuarPlanificacion					(void);
void 				bloquearESI								(char*);
void 				desbloquearESI							(char*);
void 				listarProcesos							(char*);
void 				killProceso								(char*);
void 				status									(char*);
void 				deadlock								(void);
void				salirConsola							(bool*);

/*------------------------------Auxiliares------------------------------*/
char* 				obtenerParametro						(char*, int);
bool 				estaListo								(char*);
bool 				estaBloqueadaLaClave					(char*);
bool 				estaEnLista								(char*, t_list);
bool 				sonIguales								(t_infoBloqueo);
bool 				estaBloqueadoPorLaClave					(char*, char*);
bool				estaBloqueado							(char*);
void				eliminarT_infoBloqueo					(t_infoBloqueo);

#endif /* CONSOLA_H_ */
