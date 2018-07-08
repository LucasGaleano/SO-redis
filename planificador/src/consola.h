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

/*------------------------------Globales-----------------------------*/
char* g_idESI;
char* g_clave;
bool g_estaBloqueado;
int **g_matrizAsignacion;
int **g_matrizEspera;
int g_indiceESI;
t_dictionary* g_clavesDeadlock;
t_dictionary* g_ESIsDeadlock;

typedef struct {
	int indice;
} t_infoIndiceDeadlock;

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

/*------------------------------Auxiliares-Estado de claves o ESI-----------------------------*/
bool 				estaListo								(char*);
bool 				estaBloqueadoPorLaClave					(char*, char*);
bool 				sonIguales								(t_infoBloqueo*);
bool 				estaBloqueadaLaClave					(char*);
void 				estaClaveBloqueada						(char*, t_list*);
bool 				estaBloqueadoPorElESI					(char*, char*, t_list*);
bool 				sonIgualesClavesTomadas					(char*);
bool				estaBloqueado							(char*);
void 				estaESIBloqueado						(char*, t_list*);

/*------------------------------Auxiliares-desbloquear----------------------------*/
char* 				esiQueBloquea							(char*);
void 				decirESIQueBloqueaClave					(char*, char*);
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
void 				creoElementosEnPosibleDeadlockAsignacion(char*);
void 				creoElementoEnPosibleDeadlockEspera		(char*, t_list*);
void 				creoElementosEnPosibleDeadlockEspera	(t_infoBloqueo*);
void 				creoMatrizAsignacion					(char*, t_list*);
void 				asignarEnMatrizAsignacion				(char*);
void 				creoMatrizEspera						(char*, t_list*);
void 				asignarEnMatrizEspera					(t_infoBloqueo*);
void 				esiIndice								(char*, t_infoIndiceDeadlock*);
char* 				esiQueTieneIndice						(int);

/*------------------------------Auxiliares-Liberar Memoria------------------------------*/
void 				eliminarT_infoBloqueo					(t_infoBloqueo*);

#endif /* CONSOLA_H_ */
