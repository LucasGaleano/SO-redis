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

	char* Comando = obtenerParametro(linea, 0);

	// MAN
	if (string_equals_ignore_case(Comando, "man")) {
		ejecutarMan();
		return;
	}

	// PAUSAR PLANIFICACIÓN
	if (string_equals_ignore_case(Comando, "pausar")) {
		pausarPlanificacion();
		return;
	}

	// CONTINUAR PLANIFICACIÓN
	if (string_equals_ignore_case(Comando, "continuar")) {
		continuarPlanificacion();
		return;
	}

	// BLOQUEAR
	if (string_equals_ignore_case(Comando, "bloquear")) {
		bloquear(linea);
		return;
	}

	// DESBLOQUEAR
	if (string_equals_ignore_case(Comando, "desbloquear")) {
		desbloquear(linea);
		return;
	}

	// LISTAR PROCESOS
	if (string_equals_ignore_case(Comando, "listar")) {
		listarProcesos(linea);
		return;
	}

	// KILL PROCESO
	if (string_equals_ignore_case(Comando, "kill")) {
		killProceso(linea);
		return;
	}

	// STATUS
	if (string_equals_ignore_case(Comando, "status")) {
		status(linea);
		return;
	}

	// DEADLOCK
	if (string_equals_ignore_case(Comando, "deadlock")) {
		deadlock();
		return;
	}

	// SALIR DE LA CONSOLA
	if (string_equals_ignore_case(Comando, "exit")) {
		salirConsola(ejecutar);

		return;
	}

	// NO RECONOCER COMANDO
	printf("No se ha encontrado el comando %s \n", Comando);
}

/*-------------------------------Comandos------------------------------*/
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
	printf("	deadlock -->   Muestra los procesos ESI en deadlock\n");
	printf("	exit --> Cierra la consola \n\n");
}
void salirConsola(bool* ejecutar) {
	printf("Se cerro la consola \n");
	*ejecutar = false;
}

void pausarPlanificacion(void) {
	printf("Planificador pausado.\n");
	pthread_mutex_lock(&mutexConsola);

}

void continuarPlanificacion(void) {
	printf("Planificador corriendo.\n");
	pthread_mutex_unlock(&mutexConsola);
}

void bloquear(char* linea) { // TODO se bloqueará en la próxima oportunidad posible
	char* clave = obtenerParametro(linea, 1);
	char* nombreESI = obtenerParametro(linea, 2);

	if (clave == NULL)
		return;

	if (nombreESI == NULL) {
		free(clave);
		return;
	}

	// Validaciones

	if (estaBloqueadoPorLaClave(nombreESI, clave)) {
		printf("Ya esta bloqueado el %s por la clave %s.\n", nombreESI, clave);
		return;
	}

	char* idESI = string_new();

	if (estaListo(nombreESI) || enEjecucion(nombreESI)) {
		idESI = obtenerId(nombreESI);

		if (estaBloqueadoPorElESI(clave, nombreESI)) {
			printf("El %s ya tiene la clave %s tomada.\n", nombreESI, clave);
			return;
		}

	} else {
		printf("Solo se puede bloquear el %s en estado listo o en ejecucion.\n",
				nombreESI);
		return;
	}

	// Si la clave no estaba bloqueada, se bloquea para el idESI
	if (!estaBloqueadaLaClave(clave)) {
		// Se agrega al diccionario de clavesBloqueadas por el idESI la clave a bloquear
		if (!dictionary_has_key(g_clavesTomadas, nombreESI)) {
			t_list* listaVacia = list_create();

			pthread_mutex_lock(&mutexClavesTomadas);
			dictionary_put(g_clavesTomadas, nombreESI, listaVacia);
			pthread_mutex_unlock(&mutexClavesTomadas);

		}

		pthread_mutex_lock(&mutexClavesTomadas);
		list_add(dictionary_get(g_clavesTomadas, nombreESI), clave);
		pthread_mutex_unlock(&mutexClavesTomadas);

	}

	// Si la clave esta bloqueada, se coloca al idESI en la lista de esis esperando a esa clave
	else {

		if (!dictionary_has_key(g_bloq, clave)) {
			t_list* listaVacia = list_create();

			pthread_mutex_lock(&mutexBloqueo);
			dictionary_put(g_bloq, clave, listaVacia);
			pthread_mutex_unlock(&mutexBloqueo);

		}

		// Se pasa el ESI del estado que este a Bloqueados
		t_infoBloqueo* insertar = malloc(sizeof(t_infoBloqueo));
		if (!enEjecucion(nombreESI)) {
			insertar->idESI = strdup(idESI);

			pthread_mutex_lock(&mutexListo);
			insertar->data = dictionary_remove(g_listos, idESI);

			pthread_mutex_unlock(&mutexListo);

		} else {

			insertar->idESI = strdup(g_idESIactual);
			insertar->data = g_enEjecucion; //TODO hacer copia de g_enEjecucion?? y semaforo de enEjecucion??
			// replanificar??
		}

		pthread_mutex_lock(&mutexBloqueo);
		list_add(dictionary_get(g_bloq, clave), insertar);
		pthread_mutex_unlock(&mutexBloqueo);
	}

	// Libero Memoria
	free(idESI);
}

void desbloquear(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	if (!estaBloqueadaLaClave(clave)) {
		puts("No se puede desbloquear una clave que no esta bloqueada.");
		return;
	}

	// Se saca al ESI que estaba bloqueando la clave esa clave
	char* esisQueBloqueaa = string_new();

	pthread_mutex_lock(&mutexClavesTomadas);
	esisQueBloqueaa = esiQueBloquea(clave);

	sacarClave(esisQueBloqueaa, clave);
	pthread_mutex_unlock(&mutexClavesTomadas);

	// Si alguien esta bloqueando por la clave se bloquea la clave al primer ESI, si nadie esta bloqueado por la clave solo se le saca la clave al que bloqueaba la clave
	if (dictionary_has_key(g_bloq, clave)) {
		// Se pone el primer esi en listos
		pthread_mutex_lock(&mutexBloqueo);
		t_infoBloqueo* infoBloqueoPrimero = list_remove(
				dictionary_get(g_bloq, clave), 0);
		pthread_mutex_unlock(&mutexBloqueo);

		pthread_mutex_lock(&mutexListo);
		dictionary_put(g_listos, infoBloqueoPrimero->idESI,
				infoBloqueoPrimero->data);
		pthread_mutex_unlock(&mutexListo);

		// Si no hay mas ESIs bloqueados se libera la clave
		pthread_mutex_lock(&mutexBloqueo);
		if (list_is_empty(dictionary_get(g_bloq, clave))) {
			dictionary_remove_and_destroy(g_bloq, clave, (void*) free);
		}
		pthread_mutex_unlock(&mutexBloqueo);

		// Se pone la clave en el diccionario de claves bloqueadas
		if (!dictionary_has_key(g_clavesTomadas,
				infoBloqueoPrimero->data->nombreESI)) {
			t_list* listaVacia = list_create();
			list_add(listaVacia, clave);

			pthread_mutex_lock(&mutexClavesTomadas);
			dictionary_put(g_clavesTomadas, infoBloqueoPrimero->data->nombreESI,
					listaVacia);
			pthread_mutex_unlock(&mutexClavesTomadas);
		}
		list_add(
				dictionary_get(g_clavesTomadas,
						infoBloqueoPrimero->data->nombreESI), clave);
	} else {
		free(clave);
	}

	if (list_is_empty(dictionary_get(g_clavesTomadas, esisQueBloqueaa))) {
		dictionary_remove_and_destroy(g_clavesTomadas, esisQueBloqueaa,
				(void*) free);
	}
}

void listarProcesos(char* linea) {
	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	if (!dictionary_has_key(g_bloq, clave)) {
		printf("Ningun ESI esta bloqueado por la clave %s.\n", clave);
		return;
	}

	printf("Claves de las ESIs bloqueadas por la clave %s.\n", clave);

	int i;

	pthread_mutex_lock(&mutexBloqueo);
	for (i = 0; i < list_size(dictionary_get(g_bloq, clave)); i++) {
		t_infoBloqueo* infoBloqueo = malloc(sizeof(t_infoBloqueo));
		infoBloqueo = list_get(dictionary_get(g_bloq, clave), i);

		printf("	%s \n", infoBloqueo->data->nombreESI);
	}
	pthread_mutex_unlock(&mutexBloqueo);

	// Libero memoria
	//free(clave); // creo que deberia hacer free
}

void killProceso(char* linea) {
	char* nombreESI = obtenerParametro(linea, 1);

	if (nombreESI == NULL)
		return;

	if (enEjecucion(nombreESI))
		// TODO

		// Elimino ESI en el estado que este
		pthread_mutex_lock(&mutexListo);
	if (estaListo(nombreESI)) {

		dictionary_remove_and_destroy(g_listos, obtenerId(nombreESI),
				(void*) liberarT_infoListos);

	}
	pthread_mutex_unlock(&mutexListo);

	pthread_mutex_lock(&mutexBloqueo);
	if (estaBloqueado(nombreESI)) {
		// saco al ESI de todas las listas de ESIs que están esperando esa clave
		g_nombreESI = nombreESI;
		dictionary_iterator(g_bloq, (void*) siEstaBloqueadaPorClaveEliminar);
	}
	pthread_mutex_unlock(&mutexBloqueo);

	// Liberar recursos
	pthread_mutex_lock(&mutexClavesTomadas);
	if (dictionary_has_key(g_clavesTomadas, nombreESI)) {
		// Asigno claves tomadas por otros ESIs al primer ESI que estaba esperando esa clave
		g_nombreESI = nombreESI;
		dictionary_iterator(g_clavesTomadas, (void*) desbloqueoClave);

		// Lo elimino el nombreESI del diccionario de clavesTomadas por ESI
		dictionary_remove_and_destroy(g_clavesTomadas, nombreESI, (void*) free);
	}
	pthread_mutex_unlock(&mutexClavesTomadas);

	// Libero memoria
	free(nombreESI);
}

void status(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	enviarSolicitusStatus(g_socketCoordinador, clave);

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
	g_clavesDeadlock = dictionary_create();
	g_ESIsDeadlock = dictionary_create();

	// Se calcula el numero de filas y columnas
	dictionary_iterator(g_bloq, (void*) creoElementoEnPosibleDeadlockEspera);
	dictionary_iterator(g_clavesTomadas,
			(void*) creoElementoEnPosibleDeadlockAsignacion);

	int columEsperaFilasAsig, filasEsperaColumAsig;

	filasEsperaColumAsig = dictionary_size(g_ESIsDeadlock);
	columEsperaFilasAsig = dictionary_size(g_clavesDeadlock);

	// Se crea espacio ambas matrices y las pongo en 0
	g_matrizEspera = crearMatriz(filasEsperaColumAsig, columEsperaFilasAsig);
	g_matrizAsignacion = crearMatriz(columEsperaFilasAsig,
			filasEsperaColumAsig);

	ponerMatrizTodoNulo(g_matrizEspera, filasEsperaColumAsig,
			columEsperaFilasAsig);
	ponerMatrizTodoNulo(g_matrizAsignacion, columEsperaFilasAsig,
			filasEsperaColumAsig);

	// matrizEspera W: P -> R, los procesos P estan a la espera de recursos R
	// Se pone en 1 los esis esperando en la matrizEspera
	dictionary_iterator(g_bloq, (void*) creoMatrizEspera);

	// matrizAsignacion A: R -> P, los recursos R estan a la espera de procesos P
	// Se pone en 1 las claves asignadas en la matrizAsignacion
	dictionary_iterator(g_clavesTomadas, (void*) creoMatrizAsignacion);

	// Matriz de procesosAlaEsperaDeProcesos(T) es la composicion entre matrizEspera y matrizAsignacion (T=PxA T: P -> P)
	int i, j, k;

	int procesosAlaEsperaDeProcesos[filasEsperaColumAsig][filasEsperaColumAsig];
	for (i = 0; i < filasEsperaColumAsig; i++) {
		for (j = 0; j < filasEsperaColumAsig; j++) {
			procesosAlaEsperaDeProcesos[i][j] = 0;
			for (k = 0; k < columEsperaFilasAsig; k++) {
				if (g_matrizEspera[i][k] && g_matrizAsignacion[k][j]) {
					procesosAlaEsperaDeProcesos[i][j] = 1;
					k = columEsperaFilasAsig;
				}
			}
		}
	}

	// Se calcula la matriz procesosAlaEsperaDeProcesos con cierre transitivo.
	// Se calcula aplicandole el algoritmo Warshall a la matriz procesosAlaEsperaDeProcesos
	bool hayAlgunProcesoEnDeadlock = false;
	for (k = 0; k < filasEsperaColumAsig; k++) {
		for (i = 0; i < filasEsperaColumAsig; i++) {
			for (j = 0; j < filasEsperaColumAsig; j++) {
				procesosAlaEsperaDeProcesos[i][j] =
						procesosAlaEsperaDeProcesos[i][j]
								|| (procesosAlaEsperaDeProcesos[i][k]
										&& procesosAlaEsperaDeProcesos[k][j]);
				if (i == j && procesosAlaEsperaDeProcesos[i][j])
					hayAlgunProcesoEnDeadlock = true;
			}
		}
	}

	if (hayAlgunProcesoEnDeadlock) {
		puts("\nProcesos que estan en deadlock.\n");
		int cantDeadlock = 0;
		for (i = 0; i < filasEsperaColumAsig; i++) {
			if (procesosAlaEsperaDeProcesos[i][i]) {
				printf("Deadlock: %d\n", cantDeadlock);
				cantDeadlock++;
				for (j = 0; j < filasEsperaColumAsig; j++) {
					if (procesosAlaEsperaDeProcesos[i][j]) {
						if (procesosAlaEsperaDeProcesos[j][j]) {
							printf("%s\n", esiQueTieneIndice(j));
						}
						procesosAlaEsperaDeProcesos[j][j] = 0;
					}
				}
				printf("\n");
			}
		}
	} else {
		puts("No hay ningun proceso en deadlock");
	}

	// Libero memoria
	liberarMatriz(g_matrizEspera, columEsperaFilasAsig);
	liberarMatriz(g_matrizAsignacion, filasEsperaColumAsig);
	dictionary_destroy_and_destroy_elements(g_clavesDeadlock, (void*) free);
	dictionary_destroy_and_destroy_elements(g_ESIsDeadlock, (void*) free);

}

char* obtenerParametro(char* linea, int parametro) {
	if (strcmp(linea, "") == 0)
		return "";
	char** palabras = string_split(linea, " ");
	return palabras[parametro];
}

static void decirIdESI(t_infoBloqueo* infoESI) {
	if (string_equals_ignore_case(infoESI->data->nombreESI, g_nombreESI))
		g_idESI = infoESI->idESI;
}

static void averiguarIdESIBloq(char* clave, t_list* esisBloqueados) {
	list_iterate(esisBloqueados, (void*) decirIdESI);
}

static void averiguarIdESIListos(char* idESI, t_infoListos* infoESI) {
	if (string_equals_ignore_case(infoESI->nombreESI, g_nombreESI))
		g_idESI = idESI;
}

static char* obtenerIdESIEstado(char* nombreESI, t_dictionary* diccionario,
		void (*averiguarIdESIEstado)(char*, void*)) {
	g_nombreESI = nombreESI;

	dictionary_iterator(diccionario, (void*) averiguarIdESIEstado);
	return g_idESI;
}

char* obtenerId(char* nombreESI) {

	if (enEjecucion(nombreESI))
		return g_idESIactual;
	if (estaListo(nombreESI))
		return obtenerIdESIEstado(nombreESI, g_listos,
				(void*) averiguarIdESIListos);
	else
		return obtenerIdESIEstado(nombreESI, g_bloq, (void*) averiguarIdESIBloq);
}

/*------------------------------Auxiliares-Estado de claves o ESI-----------------------------*/

static void estaListoESI(char* idESI, t_infoListos* infoESIListo) {
	if (string_equals_ignore_case(g_nombreESI, infoESIListo->nombreESI)) {
		g_bool = true;
	}

}

bool estaListo(char* nombreESI) {
	g_nombreESI = nombreESI;
	g_bool = false;

	dictionary_iterator(g_listos, (void*) estaListoESI);

	return g_bool;
}

static bool sonIguales(t_infoBloqueo* infoBloqueo) {
	return string_equals_ignore_case(infoBloqueo->data->nombreESI, g_nombreESI);
}

bool estaBloqueadoPorLaClave(char* nombreESI, char* clave) {
	if (!dictionary_is_empty(g_bloq)) {
		if (dictionary_has_key(g_bloq, clave)) {
			g_nombreESI = nombreESI;
			return list_any_satisfy(dictionary_get(g_bloq, clave),
					(void*) sonIguales);
		} else {
			return false;
		}
	} else {
		return false;
	}

}

bool estaBloqueadoPorElESI(char* clave, char* nombreESI) {
	if (!dictionary_is_empty(g_clavesTomadas)) {
		if (dictionary_has_key(g_clavesTomadas, nombreESI)) {
			g_clave = clave;
			return list_any_satisfy(dictionary_get(g_clavesTomadas, nombreESI),
					(void*) sonIgualesClaves);
		} else {
			return false;
		}
	} else {
		return false;
	}
}

static void estaClaveBloqueada(char* nombreESI, t_list* clavesBloqueadas) { // ver cuando gaston me pregunte
	if (!g_bool) {
		g_bool = estaBloqueadoPorElESI(g_clave, nombreESI);
	}
}

bool estaBloqueadaLaClave(char* clave) {
	g_clave = clave;
	g_bool = false;

	dictionary_iterator(g_clavesTomadas, (void*) estaClaveBloqueada); // ver cuando gaston me pregunte

	return g_bool;
}

bool sonIgualesClaves(char* clave) {
	return string_equals_ignore_case(clave, g_clave);
}

static void estaESIBloqueado(char* clave, t_list* esisBloqueados) {
	if (g_bool == false)
		g_bool = estaBloqueadoPorLaClave(g_nombreESI, clave);
}

bool estaBloqueado(char* nombreESI) {
	g_nombreESI = nombreESI;
	g_bool = false;

	dictionary_iterator(g_bloq, (void*) estaESIBloqueado);

	return g_bool;
}

bool enEjecucion(char* nombreESI) {
	return string_equals_ignore_case(nombreESI, g_nombreESIactual);
}

/*------------------------------Auxiliares-desbloquear----------------------------*/
static void sonIgualesLasClavesBloqueadas(char* clave) {
	if (sonIgualesClaves(clave))
		g_bool = true;
}

static void decirESIQueBloqueaClave(char* nombreESI, t_list* clavesTomadas) {
	if (!g_bool) {
		list_iterate(clavesTomadas, (void*) sonIgualesLasClavesBloqueadas);
		if (g_bool) {
			g_nombreESI = nombreESI;
		}
	}
}

char* esiQueBloquea(char* clave) {
	g_clave = clave;

	g_bool = false;
	dictionary_iterator(g_clavesTomadas, (void*) decirESIQueBloqueaClave);
	return g_nombreESI;
}

void sacarClave(char* nombreESI, char* clave) {
	g_clave = clave;
	list_remove_and_destroy_by_condition(
			dictionary_get(g_clavesTomadas, nombreESI),
			(void*) sonIgualesClaves, (void*) free);
}

/*------------------------------Auxiliares-killProceso----------------------------*/
void siEstaBloqueadaPorClaveEliminar(char* clave, t_list* listaBloqueados) {
	if (estaBloqueadoPorLaClave(g_nombreESI, clave)) {
		list_remove_and_destroy_by_condition(listaBloqueados,
				(void*) sonIguales, (void*) liberarT_infoBloqueo);
	}
	if (list_is_empty(listaBloqueados)) {
		dictionary_remove_and_destroy(g_bloq, clave, (void*) list_destroy);
	}
}

void desbloqueoClave(char* nombreESI, t_list* listaBloqueadas) {
	if (string_equals_ignore_case(nombreESI, g_nombreESI)) {
		int i;
		int tamanio = list_size(listaBloqueadas);

		for (i = 0; i < tamanio; i++) {
			char* lineaExtra = string_new();
			string_append(&lineaExtra, "desbloquear ");
			string_append(&lineaExtra, list_get(listaBloqueadas, 0));

			desbloquear(lineaExtra);

			free(lineaExtra);
		}
	}
}

/*------------------------------Auxiliares-Status------------------------------*/
void mostrarPorConsola(t_respuestaStatus* respuestaStatus) {
	if (respuestaStatus->valor == NULL)
		puts("No posee valor.");
	else
		printf("Valor: %s\n", respuestaStatus->valor);

	printf("Instancia actual en donde esta clave: %s\n",
			respuestaStatus->nomInstanciaActual);
	printf("Instancia en donde se guardaria clave: %s\n",
			respuestaStatus->nomIntanciaPosible);
}

/*------------------------------Auxiliares-Deadlock------------------------------*/

int indice(char* elemento, t_dictionary* diccionario) {
	int indice;
	if (!dictionary_has_key(diccionario, elemento)) {
		// creo una clave nueva
		indice = dictionary_size(diccionario);
		dictionary_put(diccionario, elemento, string_itoa(indice));
	} else {
		indice = atoi(dictionary_get(diccionario, elemento));
	}
	return indice;
}

int **crearMatriz(int nroFilas, int nroColum) {
	int **matriz = (int **) calloc(nroFilas, sizeof(int*));
	int i;
	for (i = 0; i < nroFilas; i++)
		matriz[i] = (int *) calloc(nroColum, sizeof(int));
	return matriz;
}

void ponerMatrizTodoNulo(int ** matriz, int nroFilas, int nroColum) {
	int i, j;
	for (i = 0; i < nroFilas; i++) {
		for (j = 0; j < nroColum; j++) {
			matriz[i][j] = 0;
		}
	}
}

static void creoElementosEnPosibleDeadlockAsignacion(char* clave) {
	indice(clave, g_clavesDeadlock);
}

void creoElementoEnPosibleDeadlockAsignacion(char* nombreESI,
		t_list* clavesBloqueadas) {
	list_iterate(clavesBloqueadas,
			(void*) creoElementosEnPosibleDeadlockAsignacion);
	indice(nombreESI, g_ESIsDeadlock);
}

static void creoElementosEnPosibleDeadlockEspera(t_infoBloqueo* esiBloqueado) {
	indice(esiBloqueado->data->nombreESI, g_ESIsDeadlock);
}

void creoElementoEnPosibleDeadlockEspera(char* clave, t_list* esisBloqueados) {
	list_iterate(esisBloqueados, (void*) creoElementosEnPosibleDeadlockEspera);
	indice(clave, g_clavesDeadlock);
}

static void asignarEnMatrizAsignacion(char* clave) {
	g_matrizAsignacion[indice(clave, g_clavesDeadlock)][indice(g_nombreESI,
			g_ESIsDeadlock)] = 1;
}

void creoMatrizAsignacion(char* nombreESI, t_list* clavesBloqueadas) {
	g_nombreESI = nombreESI;
	list_iterate(clavesBloqueadas, (void*) asignarEnMatrizAsignacion);
}

static void asignarEnMatrizEspera(t_infoBloqueo* infoESIBloqueado) {
	g_matrizEspera[indice(infoESIBloqueado->data->nombreESI, g_ESIsDeadlock)][indice(
			g_clave, g_clavesDeadlock)] = 1;
}

void creoMatrizEspera(char* clave, t_list* esisBloqueados) {
	g_clave = clave;
	list_iterate(esisBloqueados, (void*) asignarEnMatrizEspera);
}

static void esiIndice(char* nombreESI, char* nroIndice) {
	if (g_indiceESI == atoi(nroIndice))
		g_nombreESI = nombreESI;
}

char* esiQueTieneIndice(int nroIndice) {
	g_indiceESI = nroIndice;

	dictionary_iterator(g_ESIsDeadlock, (void*) esiIndice);

	return g_nombreESI;
}

/*------------------------------Auxiliares-Liberar Memoria------------------------------*/
void liberarT_infoBloqueo(t_infoBloqueo* infoBloqueo) {
	liberarT_infoListos(infoBloqueo->data);
	free(infoBloqueo->idESI);
	free(infoBloqueo);
}

void liberarT_infoListos(t_infoListos* infoListo) {
	free(infoListo->nombreESI);
	free(infoListo);
}

void liberarMatriz(int **matriz, int col) {
	//int i;

	/*for (i = 0; i < col; i++) {
	 free((*matriz)[i]);
	 }*/

	free((*matriz));
	//*matriz = 0;

}
