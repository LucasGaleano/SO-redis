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

	if (!dictionary_has_key(g_bloq, clave)) {
		t_list* listaVacia = list_create();
		dictionary_put(g_bloq, clave, listaVacia);
	}

	dictionary_remove(g_listos, id);

	// agrega al ESI a la lista de espera de esa clave
	t_infoBloqueo* infoBLoqueo = malloc(sizeof(t_infoBloqueo));
	t_infoListos infoListos = dictionary_get(g_listos, id);
	infoBLoqueo->idESI = id;
	infoBLoqueo->data = infoListos;

	t_list* lista = dictionary_get(g_bloq, clave);

	list_add(lista, infoBLoqueo);

	// Libero memoria
	free(clave);
	free(id);
	//free(lista);
	//free(infoBLoqueo);

}

void desbloquearESI(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	if (!estaBloqueadaLaClave(clave)) {
		return;
	}

	// poner el esi en listos
	t_infoBloqueo* infoBloqueoPrimero = list_get(dictionary_get(g_bloq, clave),
			0);
	dictionary_put(g_listos, infoBloqueoPrimero->idESI,
			infoBloqueoPrimero->data);

	// Desbloqueo
	list_remove(dictionary_get(g_bloq, clave), 0);

	// Libero memoria
	free(clave);
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
	char* idESI = obtenerParametro(linea, 1);

	if (idESI == NULL)
		return;

	// libero las claves que tiene tomado el ESI kasefjbskj

	// preguntar lucas: como se que claves tiene bloqueado un esi
	// esto que escribo es conceptual, cuando lo hable con lucas adaptarlo
	if (tieneAlgunaClaveBloqueda(idESI)) {
		char* clavesBloqueadasPorEsi[]; // se que esta mal escrito
		int i;
		for (i = 0; i < cantClavesBloqueadas; i++) {
			char* lineaExtra = string_new();
			string_append(&lineaExtra, "desbloquear ");
			string_append(&lineaExtra, clavesBloqueadasPorEsi[i]);
			desbloquearESI(lineaExtra);
		}
	}

	// free(en el estado que este)
	if (estaListo(idESI)) {
		dictionary_remove_and_destroy(g_listos, idESI, (void*) free);
	}

	if (estaBloqueado(idESI)) {
		char* clavesBloqueadasPorEsi[]; // se que esta mal escrito
		int i;

		for (i = 0; i < cantClavesBloqueadas; i++) {
			if (estaBloqueadoPorElRecurso(idESI, clavesBloqueadasPorEsi[i])) {
				t_list* lista = dictionary_get(g_bloq, claveRecurso);
				g_idComparar = idESI;
				list_remove_and_destroy_by_condition(lista, (void*) sonIguales,
						(void*) eliminarT_infoBloqueo);
			}

		}
	}

	// Libero memoria
	free(idESI);
}

void status(char* linea) {
	// informacion sobre las instancias del sistema
	// PREGUNTA

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	// comunicarse con el coordinador
	// hablar con lucas

	// esto solo es conceptual
	if(/*TengoValor*/)
		puts("Valor: %", );
	else
		puts("No posee valor.");

	puts("Instancia actual en donde esta clave: %", );
	puts("Instancia en donde se guardaria clave: %", );

	// muestro ESIs bloqueados a la espera de dicha clave
	char* lineaExtra = string_new();
	string_append(&lineaExtra, "listar ");
	string_append(&lineaExtra, clave);
	listarProcesos(lineaExtra);

	// Libero memoria
	free(clave);
	free(lineaExtra);
}

void deadlock() {
	/*t_list** ESIsEnDeadlock = devolverEsisEnDeadlock()*/
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

bool estaBloqueadaLaClave(char* clave) {
	return dictionary_has_key(clave);
}

bool sonIguales(t_infoBloqueo infoBloqueo) {
	return string_equals_ignore_case(infoBloqueo->idESI, g_idComparar);
}

bool estaBloqueadoPorLaClave(char* id, char* clave) {
	g_idComparar = id;
	return list_any_satisfy(dictionary_get(g_bloq, clave), (void*) sonIguales);
}

bool estaBloqueado(char* id) {
	int i;

	for (i = 0; i < g_bloq->elements_amount; i++) {

		char* numero = string_itoa(i); // repitiendo logica, arreglar desp
		char* claveESI = string_new();

		string_append(&claveESI, "ESI");
		string_append(&claveESI, numero);

		if (estaBloqueadoPorElRecurso(claveESI, claveRecurso)) {
			return true;
		}

		free(claveESI);
	}
	return false;
}

void eliminarT_infoBloqueo(t_infoBloqueo infoBloqueo) {
	free(infoBloqueo->data);
	free(infoBloqueo->idESI);
	free(infoBloqueo);
}
