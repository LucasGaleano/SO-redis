/*
 * consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "consola.h"

/*------------------------------Consola------------------------------*/
void iniciarConsola() {
	char* linea;
	bool ejecutar = true;

	while (ejecutar) {
		linea = readline(">");

		if (linea) {
			add_history(linea);
		} else {
			free(linea);
			break;
		}

		ejecutarComando(linea, &ejecutar);

		free(linea);
	}

	clear_history();
}

void ejecutarComando(char* linea, bool* ejecutar) {

	// MAN
	if (string_equals_ignore_case(linea, "man")) {
		ejecutarMan();
		return;
	}

	// PAUSAR PLANIFICACIÓN
	if (string_equals_ignore_case(linea, "pausar")) {
		pausarPlanificacion();
		return;
	}

	// CONTINUAR PLANIFICACIÓN
	if (string_equals_ignore_case(linea, "continuar")) {
		continuarPlanificacion();
		return;
	}

	// BLOQUEAR ESI
	if (string_equals_ignore_case(linea, "bloquear")) {
		bloquearESI(linea);
		return;
	}

	// DESBLOQUEAR ESI
	if (string_equals_ignore_case(linea, "desbloquear")) {
		desbloquearESI(linea);
		return;
	}

	// LISTAR PROCESOS
	if (string_equals_ignore_case(linea, "listar")) {
		listarProcesos(linea);
		return;
	}

	// KILL PROCESO
	if (string_equals_ignore_case(linea, "kill")) {
		killProceso(linea);
		return;
	}

	// STATUS
	if (string_equals_ignore_case(linea, "status")) {
		status(linea);
		return;
	}

	// DEADLOCK
	if (string_equals_ignore_case(linea, "deadlock")) {
		deadlock();
		return;
	}

	// SALIR DE LA CONSOLA
	if (string_equals_ignore_case(linea, "exit")) {
		salirConsola(ejecutar);
		return;
	}

	// NO RECONOCER COMANDO
	printf("No se ha encontrado el comando %s \n", linea);
}

/*------------------------------Comandos------------------------------*/
void ejecutarMan() {
	printf("NAME \n");
	printf("	consola \n\n");

	printf("SYNOPSIS \n");
	printf("	#include <consola.h> \n\n");

	printf("	void pausar(void) \n");
	printf("	void continuar(void) \n");
	printf("	void bloquear(char* clave, int id) \n");
	printf("	void desbloquear(char* clave, int id ) \n");
	printf("	void listar(char* recurso) \n");
	printf("	void kill(char* clave, int id) \n");
	printf("	void status(char* clave) \n");
	printf("	void deadlock(void) \n");
	printf("	void exit(void) \n\n");

	printf("DESCRIPTION \n");
	printf("	pausar --> Pausar planificacion \n");
	printf("	continuar --> Continuar planificacion \n");
	printf(
			"	bloquear --> Se bloqueara el proceso ESI hasta ser desbloqueado \n");
	printf(
			"	desbloquear -->  Se desbloqueara el proceso ESI con el ID especificado \n");
	printf("	listar --> Lista los procesos encolados esperando al recurso \n");
	printf("	kill --> Finaliza el proceso ESI \n");
	printf(
			"	status -->  Brinda informacion sobre las instancias del sistema \n");
	printf("	deadlock -->   \n"); // Ver
	printf("	exit --> Cierra la consola \n\n");
}
void salirConsola(bool* ejecutar) {
	printf("Se cerro la consola \n");
	*ejecutar = false;
}

void pausarPlanificacion(void) {
	pthread_mutex_lock(&mutexConsola);
	// PREGUNTA
	// preguntar gaston poner en planificador.h
	//pthread_mutex_t mutexConsola = PTHREAD_MUTEX_INITIALIZER;
}

void continuarPlanificacion(void) {
	pthread_mutex_unlock(&mutexConsola);
}

void bloquearESI(char* linea) {
	char* clave = obtenerParametro(linea, 1);
	char* id = obtenerParametro(linea, 2);

	if (clave == NULL)
		return;

	if (id == NULL) {
		free(clave);
		return;
	}
	//int id = atoi(id_char); // Hacer atoi cuando lo use

	if (!(estaListo(clave) || estaEjecutando(clave))) {
		puts("Solo se puede bloquear el ESI en estado listo o ejecutando.");
		return;
	}

	if (estaListo(clave)) {
		free(dictionary_remove(g_listos, clave));
	}

	if (estaEjecutando(clave)) {
		free(dictionary_remove(g_exec, clave));
	}

	// falta distinguir si esta bloqueado por la consola no?
	/*t_infoBloqueo* aux;			PREGUNTA
	 aux->bloqueoUsuario = true;*/

	// Bloquear proceso
	dictionary_put(g_bloq, clave, NULL);

	// Libero memoria
	free(clave);
	free(id);

}

void desbloquearESI(char* linea) {

	char* clave = obtenerParametro(linea, 1);
	char* id = obtenerParametro(linea, 2);

	if (clave == NULL)
		return;

	if (id == NULL) {
		free(clave);
		return;
	}

	if (!estaBloqueado(clave)) {
		printf("No se puede desbloquear un ESI que no está bloqueado \n");
		return;
	}

	if (!estaBloqueadoPorUsuario(clave)) {
		printf(
				"Solo se puede desbloquear un ESI que fue bloqueado por la consola \n");
		return;
	}

	// Desbloqueo
	free(dictionary_remove(g_bloq, clave));
	dictionary_put(g_listos, clave, NULL);

	// Libero memoria
	free(clave);
	free(id);

}

void listarProcesos(char* linea) {
	char* claveRecurso = obtenerParametro(linea, 1);

	if (claveRecurso == NULL)
		return;

	printf("Claves de las ESIs bloqueadas por el recurso\n");

	int i;

	for (i = 0; i < g_bloq->elements_amount; i++) {

		char* numero = string_itoa(i);
		char* claveESI = string_new();

		string_append(&claveESI, "ESI");
		string_append(&claveESI, numero);

		if (estaBloqueadoPorElRecurso(claveESI, claveRecurso)) {
			printf("	%s \n", claveESI);
		}

		free(claveESI);
	}

	// Libero memoria
	free(claveRecurso);
}

void killProceso(char* linea) {
	char* clave = obtenerParametro(linea, 1);
	char* id = obtenerParametro(linea, 2);

	if (clave == NULL)
		return;

	if (id == NULL) {
		free(clave);
		return;
	}

	// PREGUNTAR que lo de atomicidad
	// recordar la atomicidad en bloquear

	// PREGUNTA si est{a bien esto despues de pensarlo mas
	dictionary_put(g_term, clave, NULL);


	// liberar recursos?

	/*No se si sirve
	 * liberar mapa de memoria
	 * cerrar ficheros y liberar otros recursos
	 * eliminar PCB de cola de procesos listos
	 * liberar PCB
	 * liberar pila del sistema
	 * activa planificador y reliza c. de contexto al proceso elegido*/

	// Libero memoria
	free(clave);
	free(id);
}

void status(char* linea) {
	// informacion sobre las instancias del sistema
	// PREGUNTA
	// es clave de la instancia? y la clave seria el nombre de la instancia?

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	// comunicarse con el coordinador PREGUNTA4

	printf("Informacion de la instancia \n");
	printf("	Path del archivo de conguracion: %s \n", /*instancia.path*/); // ?? t_config * archivoConfig;
	printf("	Coordinador del puerto de config: %d \n", /*instancia.coordinadorPuertoConfig*/);
	printf("	Coordinador Ip de Config: %s \n", /*instancia.coordinadorIpConfig*/);
	printf("	Algoritmo de reemplazo: %s \n", /*instancia.algoritmoReemplazo*/);
	printf("	Punto de montaje: %s \n", /*instancia.puntoMontaje*/);
	printf("	Nombre de instancia: %s \n", /*instancia.nombreInstancia*/);
	printf("	Intervalo Dump: %s \n", /*instancia.puntoMontaje*/);

	// Libero memoria
	free(clave);
}

void deadlock() {
	// Mostrar los ESIs que estan en un deadlock? PREGUNTA6

	// Ver en bloqueos enunciado (p 9)
	// El Planificador lleva un registro de qué claves fueron bloqueadas por cada ESI en particular. Las cuales deberá liberar en cuanto reciba una operación STORE con dicha clave por parte de la ESI bloqueadora
	// Esta liberación será de manera FIFO; el primer ESI que se encontraba bloqueado esperando esta clave será liberada (Esto no quiere decir que será inmediatamente tomado por este ESI; sino que estará disponible para ser planificado; y deberá re ejecutar la operación de GET al ser ejecutado).
	// Cabe aclarar que la finalización de un ESI libera los recursos que este tenía tomados.
}

/*------------------------------Auxiliares------------------------------*/

char* obtenerParametro(char* linea, int parametro) {
	char** palabras = string_split(linea, " ");
	return palabras[parametro];
}

bool estaListo(char* clave) {
	return dictionary_has_key(g_listos, clave);
}

bool estaEjecutando(char* clave) {
	return dictionary_has_key(g_exec, clave);
}

bool estaBloqueado(char* clave) {
	return dictionary_has_key(g_bloq, clave);
}

bool estaBloqueadoPorUsuario(char* clave) {
	/*t_infoBloqueo *data = dictionary_get(g_bloq, clave);
	 return data->bloqueoUsuario;*/
}

bool estaBloqueadoPorElRecurso(char* claveESI, char* claveRecurso) {
	/*t_infoBloqueo* infoESI = dictionary_get(g_bloq, claveESI);

	 return string_equals_ignore_case(infoESI->codRecurso, claveRecurso);*/
}

