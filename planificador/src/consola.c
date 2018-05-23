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

	if (!(estaListo(id))) {
		puts("Solo se puede bloquear el ESI en estado listo.");
		return;
	}

	if (estaBloqueadoPorLaClave(id, clave)) {
		puts("Ya esta bloqueado el ESI por la clave.");
		return;
	}

	free(dictionary_remove(g_listos, id)); //preg casti
	dictionary_remove_and_destroy(g_listos, id,
			(void*) list_remove_and_destroy_element);

	if (!dictionary_has_key(g_bloq, clave)) {
		t_list listaVacia = list_create();
		dictionary_put(g_bloq, clave, listaVacia);
	}

	/*agrega al ESI a la lista de espera de esa clave*/
	t_list lista = dictionary_get(g_bloq, clave);
	t_infoBloqueo infoBLoqueo = malloc(sizeof(t_infoBloqueo));
	infoBLoqueo->idESI = id;
	infoBLoqueo->data = NULL; // preguntar casti porque no esta bien
	list_add(lista, infoBLoqueo);

	// Libero memoria
	free(clave);
	free(id);
	free(lista);
	free(infoBLoqueo);

}

void desbloquearESI(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	if (!estaBloqueado(clave)) {
		return;
	}

	t_infoBloqueo infoBloqueoPrimero = list_get(g_bloq, 0);
	// poner el esi en listos
	dictionary_put(g_listos, infoBloqueoPrimero->idESI, infoBloqueoPrimero->data);

	// Desbloqueo
	g_idComparar = infoBloqueoPrimero->idESI;
	list_remove_and_destroy_by_condition(g_bloq,(void*)sonIguales,(void*)/*¿¿*/); //preguntar

	/*// Desbloqueo
	 free(dictionary_remove(g_bloq, clave));*/

	// Libero memoria
	free(clave);
	// free(infoBloqueoPrimero); // se que no es asi, es solo para poner la idea

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
	// free(en el estado que este)
	// liberar todas las claves tomadas por el esi
	// puede que haya que desbloquear alguna clave

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

	//printf("Informacion de la instancia \n");
	//printf("	Path del archivo de conguracion: %s \n", /*instancia.path*/); // ?? t_config * archivoConfig;
	//printf("	Coordinador del puerto de config: %d \n", /*instancia.coordinadorPuertoConfig*/);
	//printf("	Coordinador Ip de Config: %s \n", /*instancia.coordinadorIpConfig*/);
	//printf("	Algoritmo de reemplazo: %s \n", /*instancia.algoritmoReemplazo*/);
	//printf("	Punto de montaje: %s \n", /*instancia.puntoMontaje*/);
	//printf("	Nombre de instancia: %s \n", /*instancia.nombreInstancia*/);
	//printf("	Intervalo Dump: %s \n", /*instancia.puntoMontaje*/);

	dictionary_has_key(g_bloq, clave);

	// Libero memoria
	free(clave);
}

void deadlock() {
	/*t_list* ESIsEnDeadlock = devolverEsisEnDeadlock()*/
	/*
	 * printf("ESIs en deadlock \n");
	 *
	 * while(termino de leer toda la lista)
	 * {
	 * 	printf("%s \n", claveESI);
	 * }*/

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

bool estaListo(char* id) {
	return dictionary_has_key(g_listos, id);
}

bool estaBloqueado(char* clave) {
	return dictionary_has_key(clave);
}

bool sonIguales(t_infoBloqueo infoBloqueo) {
	return string_equals_ignore_case(infoBloqueo->idESI, g_idComparar);
}

bool estaBloqueadoPorLaClave(char* id, char* clave) {
	g_idComparar = id;
	return list_any_satisfy(dictionary_get(g_bloq, clave), (void*) sonIguales);
}
