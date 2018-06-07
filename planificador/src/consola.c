/*
 * consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

// no puse semaforos de g_clavesBloqueadas
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
	printf("	deadlock -->   Muestra los procesos ESI en deadlock\n");
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
	char* idESI = obtenerParametro(linea, 2);

	if (clave == NULL)
		return;

	if (idESI == NULL) {
		free(clave);
		return;
	}

	if (!(estaListo(idESI))) {
		puts("Solo se puede bloquear el ESI en estado listo.");
		return;
	}

	if (estaBloqueadoPorLaClave(idESI, clave)) {
		puts("Ya esta bloqueado el ESI por la clave.");
		return;
	}

	// agrega al ESI a la lista de espera de esa clave
	pthread_mutex_lock(&mutexBloqueo);
	if (!dictionary_has_key(g_bloq, clave)) {
		t_list* listaVacia = list_create();
		dictionary_put(g_bloq, clave, listaVacia);
	}
	pthread_mutex_unlock(&mutexBloqueo);

	t_infoBloqueo* infoBloqueo = malloc(sizeof(t_infoBloqueo));
	infoBloqueo->idESI = idESI;
	pthread_mutex_lock(&mutexListo);
	infoBloqueo->data = dictionary_remove(g_listos, idESI);
	pthread_mutex_unlock(&mutexListo);

	pthread_mutex_lock(&mutexBloqueo);
	list_add(dictionary_get(g_bloq, clave), infoBloqueo);
	pthread_mutex_unlock(&mutexBloqueo);

	// Libero memoria
	free(clave);
	free(idESI);
	eliminarT_infoBLoqueo(infoBloqueo);

}

void desbloquearESI(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	// poner el esi en listos
	pthread_mutex_lock(&mutexBloqueo);
	t_infoBloqueo* infoBloqueoPrimero = list_remove(
			dictionary_get(g_bloq, clave), 0);
	pthread_mutex_unlock(&mutexBloqueo);

	pthread_mutex_lock(&mutexListo);
	dictionary_put(g_listos, infoBloqueoPrimero->idESI,
			infoBloqueoPrimero->data);
	pthread_mutex_unlock(&mutexListo);

	// pongo la clave en el diccionario de claves bloqueadas
	if (!dictionary_has_key(g_clavesBloqueadas, infoBloqueoPrimero->idESI)) {
		t_list* listaVacia = list_create();
		list_add(listaVacia, clave);
		dictionary_put(g_clavesBloqueadas, infoBloqueoPrimero->idESI,
				listaVacia);
		free(listaVacia);
	}
	list_add(dictionary_get(g_clavesBloqueadas, infoBloqueoPrimero->idESI),
			clave);

	// Libero memoria
	free(clave);
	eliminarT_infoBLoqueo(infoBloqueoPrimero);
}

void listarProcesos(char* linea) {
	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	printf("Claves de las ESIs bloqueadas por el recurso\n");

	int i;

	pthread_mutex_lock(&mutexBloqueo);
	for (i = 0; i < g_bloq->elements_amount; i++) {

		char* numero = string_itoa(i);
		char* idESI = string_new();

		string_append(&idESI, "ESI");
		string_append(&idESI, numero);

		if (estaBloqueadoPorElRecurso(idESI, clave)) {
			printf("	%s \n", idESI);
		}

		free(idESI);
	}
	pthread_mutex_unlock(&mutexBloqueo);

	// Libero memoria
	free(clave);
}

void killProceso(char* linea) {
	char* idESI = obtenerParametro(linea, 1);

	if (idESI == NULL)
		return;

	// Elimino ESI en el estado que este
	pthread_mutex_lock(&mutexListo);
	if (estaListo(idESI)) {
		dictionary_remove_and_destroy(g_listos, idESI, (void*) free);
	}
	pthread_mutex_unlock(&mutexListo);

	pthread_mutex_lock(&mutexBloqueo);
	if (estaBloqueado(idESI)) {
		// saco al ESI de todas las listas de ESIs que están esperando esa clave
		g_idESI = idESI;
		dictionary_iterator(g_bloq, (void*) siEstaBloqueadaPorClaveEliminar);
	}
	pthread_mutex_unlock(&mutexBloqueo);

	// Liberar recursos
	if (dictionary_has_key(g_clavesBloqueadas, idESI)) {
		// Asigno claves tomadas por otros ESIs al primer ESI que estaba esperando esa clave
		dictionary_iterator(g_clavesBloqueadas, (void*) desbloqueoClave);

		// Lo elimino el idESI del diccionario de claves bloqueadas por ESI
		dictionary_remove_and_destroy(g_clavesBloqueadas, idESI,
				(void*) eliminarT_infoClavesBloqueadas);
	}

	// Libero memoria
	free(idESI);
}

void status(char* linea) {

	char* clave = obtenerParametro(linea, 1);

	if (clave == NULL)
		return;

	enviarSolicitusStatus(g_socketCoordinador, clave); // decirle a facu que falta en paquetes.h

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
	int i, j, k;
	int columEsperaFilasAsig, filasEsperaColumAsig;
	g_clavesDeadlock = dictionary_create();
	g_ESIsDeadlock = dictionary_create();

	// Se calcula el numero de filas y columnas
	dictionary_iterator(g_bloq, (void*) creoElementoEnPosibleDeadlockEspera);
	dictionary_iterator(g_clavesBloqueadas,
			(void*) creoElementoEnPosibleDeadlockAsignacion);

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
	dictionary_iterator(g_clavesBloqueadas, (void*) creoMatrizAsignacion);

	// Matriz de procesosAlaEsperaDeProcesos(T) es la composicion entre matrizEspera y matrizAsignacion () T=PxA T: P -> P
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
	for (k = 0; k < filasEsperaColumAsig; k++) {
		for (i = 0; i < filasEsperaColumAsig; i++) {
			for (j = 0; j < filasEsperaColumAsig; j++) {
				procesosAlaEsperaDeProcesos[i][j] =
						procesosAlaEsperaDeProcesos[i][j]
								|| (procesosAlaEsperaDeProcesos[i][k]
										&& procesosAlaEsperaDeProcesos[k][j]);
			}
		}
	}

	puts("\nProcesos que estan en deadlock por ciclo\n");
	int cantDeadlock = 0;
	for (i = 0; i < filasEsperaColumAsig; i++) {
		if (procesosAlaEsperaDeProcesos[i][i]) {
			printf("Ciclo: %d\n", cantDeadlock);
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
}

/*------------------------------Auxiliares------------------------------*/

char* obtenerParametro(char* linea, int parametro) {
	char** palabras = string_split(linea, " ");
	return palabras[parametro];
}

bool estaListo(char*id) {
	return dictionary_has_key(g_listos, id);
}

bool estaBloqueadoPorLaClave(char* idESI, char* clave) {
	g_idESI = idESI;
	return list_any_satisfy(dictionary_get(g_bloq, clave), (void*) sonIguales);
}

bool sonIguales(t_infoBloqueo infoBloqueo) {
	return string_equals_ignore_case(infoBloqueo->idESI, g_idESI);
}

bool estaBloqueadaLaClave(char* clave) {
//repito logica, arreglar despues
	g_idESI = clave;
	g_estaBloqueado = 0;

	dictionary_iterator(g_clavesBloqueadas, (void*) estaClaveBloqueada);

	return g_estaBloqueado;
}

void estaClaveBloqueada(char* idESI, t_list* clavesBloqueadas) {
	if (!g_estaBloqueado) {
		g_estaBloqueado = estaBloqueadoPorElESI(g_idESI, idESI,
				clavesBloqueadas);
	}
}

void estaESIBloqueado(char* clave, t_list* esisBloqueados) {
	if (g_estaBloqueado == false)
		g_estaBloqueado = estaBloqueadoPorLaClave(g_idESI, clave);
}

bool estaBloqueado(char* idESI) {
	g_idESI = idESI;
	g_estaBloqueado = false;

	dictionary_iterator(g_bloq, (void*) estaESIBloqueado);

	return g_estaBloqueado;
}

void eliminarT_infoBloqueo(t_infoBloqueo* infoBloqueo) {
	free(infoBloqueo->data);
	free(infoBloqueo->idESI);
	free(infoBloqueo);
}

void siEstaBloqueadaPorClaveEliminar(char* clave, t_list* listaBloqueados) {
	if (estaBloqueadoPorLaClave(g_idESI, clave)) {
		list_remove_and_destroy_by_condition(listaBloqueados,
				(void*) sonIguales, (void*) eliminarT_infoBloqueo);
	}
}

void desbloqueoClave(char* idESI, t_list* listaBloqueadas) {
	int i;
	int tamañoLista = list_size(listaBloqueadas);
	for (i = 0; i < tamañoLista; i++) {
		char* lineaExtra = string_new();
		string_append(&lineaExtra, "desbloquear ");
		string_append(&lineaExtra, list_get(listaBloqueadas, i));
		desbloquearESI(lineaExtra);

		free(lineaExtra);
	}
}

void eliminarT_infoClavesBloqueadas(
		t_infoClavesBloqueadas* infoClavesBloqueadas) {
	free(infoClavesBloqueadas->clave);
	free(infoClavesBloqueadas);
}

void mostrarPorConsola(t_respuestaStatus* respuestaStatus) {
	if (respuestaStatus->valor == NULL)
		puts("No posee valor.");
	else
		puts("Valor: %s", respuestaStatus->valor);

	puts("Instancia actual en donde esta clave: %s",
			respuestaStatus->nomInstanciaActual);
	puts("Instancia en donde se guardaria clave: %s",
			respuestaStatus->nomIntanciaPosible);
}

void eliminarT_infoBLoqueo(t_infoBloqueo* infoBloqueo) {
	free(infoBloqueo->data);
	free(infoBloqueo->idESI);
	free(infoBloqueo);
}

int indice(char* elemento, t_dictionary* diccionario) {
	// creo una clave nueva
	t_infoIndiceDeadlock* infoElemento = malloc(sizeof(t_infoIndiceDeadlock));
	if (!dictionary_has_key(diccionario, elemento)) {
		infoElemento->indice = dictionary_size(diccionario);
		dictionary_put(diccionario, elemento, infoElemento);
	} else {
		infoElemento = dictionary_get(diccionario, elemento);
	}
	return infoElemento->indice;
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
			printf("| %d |", matriz[i][j]);
		}
		printf("\n");
	}
}

void creoElementoEnPosibleDeadlockAsignacion(char* idESI,
		t_list* clavesBloqueadas) {
	// repito logica
	g_idESI = idESI;
	list_iterate(clavesBloqueadas,
			(void*) creoElementosEnPosibleDeadlockAsignacion);
}

void creoElementosEnPosibleDeadlockAsignacion(
		t_infoClavesBloqueadas* infoClavesBloqueada) {
	indice(infoClavesBloqueada->clave, g_clavesDeadlock);
	indice(g_idESI, g_ESIsDeadlock);
}

void creoElementoEnPosibleDeadlockEspera(char* idElemento,
		t_list* clavesBloqueadas) {
	// repito logica
	g_idESI = idElemento;
	list_iterate(clavesBloqueadas,
			(void*) creoElementosEnPosibleDeadlockEspera);
}

void creoElementosEnPosibleDeadlockEspera(t_infoBloqueo* esiBloqueado) {
	indice(g_idESI, g_clavesDeadlock);
	indice(esiBloqueado->idESI, g_ESIsDeadlock);
}

void creoMatrizAsignacion(char* idESI, t_list* clavesBloqueadas) {
	g_idESI = idESI;
	list_iterate(clavesBloqueadas, (void*) asignarEnMatrizAsignacion);
}

void asignarEnMatrizAsignacion(t_infoClavesBloqueadas* infoClavesBloqueada) {
	g_matrizAsignacion[indice(infoClavesBloqueada->clave, g_clavesDeadlock)][indice(g_idESI, g_ESIsDeadlock)] =
				1;
}

void creoMatrizEspera(char* clave, t_list* esisBloqueados) {
	g_idESI = clave;
	list_iterate(esisBloqueados, (void*) asignarEnMatrizEspera);
}

void asignarEnMatrizEspera(t_infoBloqueo* infoESIBloqueado) {
	g_matrizEspera[indice(
			 infoESIBloqueado->idESI, g_ESIsDeadlock)][indice(g_idESI, g_clavesDeadlock)] = 1;
}

void esiIndice(char* idESI, t_infoIndiceDeadlock* infoESI) {

	if (g_indiceESI == infoESI->indice)
		g_idESI = idESI;

}

char* esiQueTieneIndice(int indice) {
	g_indiceESI = indice;
	dictionary_iterator(g_ESIsDeadlock, (void*) esiIndice);
	return g_idESI;
}


