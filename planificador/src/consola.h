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
#include <biblioteca/estructuras.h>
#include "globales.h"
#include "algoritmos.h"

/*------------------------------Globales-----------------------------*/
char* g_nombreESI;
char* g_clave;
char* g_idESI;
bool g_bool;
int **g_matrizAsignacion;
int **g_matrizEspera;
int g_indiceESI;

t_dictionary* g_clavesDeadlock;
t_dictionary* g_ESIsDeadlock;


/*------------------------------Consola------------------------------*/
void 				iniciarConsola							(void);
void 				ejecutarComando							(char *, bool *);

/*------------------------------Comandos------------------------------*/
void 				ejecutarMan								(void);
void 				pausarPlanificacion						(void);
void 				continuarPlanificacion					(void);
void 				bloquear								(char*);
void 				desbloquear								(char*);
void 				listarProcesos							(char*);
void 				killProceso								(char*);
void 				status									(char*);
void 				deadlock								(void);
void				salirConsola							(bool*);

/*------------------------------Auxiliares------------------------------*/
char* 				obtenerParametro						(char*, int);
char* 				obtenerId								(char*);

/*------------------------------Auxiliares-Estado de claves o ESI-----------------------------*/
bool 				estaListo								(char*);
bool 				estaBloqueadoPorLaClave					(char*, char*);
bool 				estaBloqueadaLaClave					(char*);
bool 				estaBloqueadoPorElESI					(char*, char*);
bool 				sonIgualesClaves						(char*);
bool				estaBloqueado							(char*);
bool 				enEjecucion								(char*);

/*------------------------------Auxiliares-desbloquear----------------------------*/
char* 				esiQueBloquea							(char*);
void 				sacarClave								(char*, char*);

/*------------------------------Auxiliares-killProceso----------------------------*/
void        		siEstaBloqueadaPorClaveEliminar			(char*, t_list*);
void        		desbloqueoClave							(char*, t_list*);

/*------------------------------Auxiliares-Status------------------------------*/
void 				mostrarPorConsola						(t_respuestaStatus*);

/*------------------------------Auxiliares-Deadlock------------------------------*/
int 				indice									(char*, t_dictionary*);
int **				crearMatriz								(int, int);
void 				ponerMatrizTodoNulo						(int **, int, int);
void 				creoElementoEnPosibleDeadlockAsignacion	(char* idESI, t_list* clavesBloqueadas);
void 				creoElementoEnPosibleDeadlockEspera		(char*, t_list*);
void 				creoMatrizAsignacion					(char*, t_list*);
void 				creoMatrizEspera						(char*, t_list*);
char* 				esiQueTieneIndice						(int);

/*------------------------------Auxiliares-Liberar Memoria------------------------------*/
void 				liberarT_infoBloqueo					(t_infoBloqueo*);
void 				liberarT_infoListos						(t_infoListos*);
void 				liberarMatriz							(int **, int);

#endif /* CONSOLA_H_ */
