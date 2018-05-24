#include "algoritmos.h"

double calcularProximaRafaga(double estimadoAnterior,
		double realAnterior) {
	return estimadoAnterior * g_alfa + realAnterior * (1 - g_alfa);
}

double calcularRR(double tEnEspera, double estimadoAnterior,
		double realAnterior) {
	return (1
			+ tEnEspera / calcularProximaRafaga(estimadoAnterior, realAnterior));
}

char* asignarID(int val, char* ret) {
	char* num = string_itoa(val);
	ret = malloc(strlen(num) + 3);
	strcpy(ret, "ESI");
	return strcat(ret, num);
}

void bloquear(t_infoListos* bloq, int nuevoReal, char* key) {
	bloq->estAnterior = calcularProximaRafaga(bloq->estAnterior,
			bloq->realAnterior);
	bloq->realAnterior = nuevoReal;
	t_infoBloqueo* insert = malloc(sizeof(t_infoBloqueo));
	insert->idESI = strdup(key);
	insert->data = bloq;
	pthread_mutex_lock(&mutexBloqueo);
	if (dictionary_has_key(g_bloq, g_claveGET)) {
		list_add(dictionary_get(g_bloq, g_claveGET), insert);
	} else {
		t_list* aux = list_create();
		list_add(aux, insert);
		dictionary_put(g_bloq, g_claveGET, aux);
	}
	pthread_mutex_unlock(&mutexBloqueo);
}

char* calcularSiguienteSJF(void) {
	t_infoListos *actual;
	double menor;

	int i = 0;
	char* auxKey = asignarID(i, auxKey);
	char* keyMenor = calloc(5, sizeof(char));
	actual = dictionary_get(g_listos, auxKey);
	menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
	keyMenor = strdup(auxKey);
	for (i++; i < g_listos->elements_amount; i++) {
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		double prox = calcularProximaRafaga(actual->estAnterior,
				actual->realAnterior);
		if (menor < prox) {
			menor = prox;
			strcpy(keyMenor, auxKey);
		}
	}

	return keyMenor;
}

char* calcularSiguienteHRRN(void) {
	t_infoListos *actual;
	double mayor;

	int i = 0;
	char* auxKey = asignarID(i, auxKey);
	char* keyMayor;
	actual = dictionary_get(g_listos, auxKey);
	mayor = calcularRR(actual->tEnEspera, actual->estAnterior,
			actual->realAnterior);
	keyMayor = strdup(auxKey);
	for (i++; i < g_listos->elements_amount; i++) {
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		double prox = calcularRR(actual->tEnEspera, actual->estAnterior,
				actual->realAnterior);
		if (mayor < prox) {
			mayor = prox;
			strcpy(keyMayor, auxKey);
		}
	}

	return keyMayor;
}

void envejecer(char* key, t_infoListos* data) {
	data->tEnEspera += g_tEjecucion;
}

void planificarSinDesalojo(char* algoritmo) {

	int cont;
	t_infoListos *aEjecutar;
	char* key;
	while (1) {
		cont = 0;
		pthread_mutex_lock(&mutexListo);
		pthread_cond_wait(&ESIentrada, &mutexListo);
		if (strcmp(algoritmo, "SJF-SD") == 0)
			key = calcularSiguienteSJF();
		if (strcmp(algoritmo, "HRRN") == 0)
			key = calcularSiguienteHRRN();

		aEjecutar = dictionary_remove(g_listos, key);
		pthread_mutex_unlock(&mutexListo);
		g_socketEnEjecucion = aEjecutar->socketESI;
		while (!g_termino && !g_bloqueo) {
			pthread_mutex_lock(&mutexConsola);
			enviarSolicitudEjecucion(g_socketEnEjecucion);
			pthread_mutex_unlock(&mutexConsola);
			gestionarSolicitudes(g_socketEnEjecucion,
					(void*) gestionarRespuestaESI, g_logger);
			if (!g_termino) {
				gestionarSolicitudes(g_socketCoordinador,
						(void*) gestionarRespuestaCoordinador, g_logger);
				cont++;
			}
		}
		if (g_bloqueo) {
			g_bloqueo = 0;
			bloquear(aEjecutar, cont, key);
		}
		if (g_termino) {
			g_termino = 0;
			free(aEjecutar);
		}
		if (strcmp(algoritmo, "HRRN") == 0) {
			g_tEjecucion = cont;
			pthread_mutex_lock(&mutexListo);
			dictionary_iterator(g_listos, (void*) envejecer);
			pthread_mutex_unlock(&mutexListo);
		}
		free(key);
	}
}

void planificarConDesalojo(void) {
	int cont;
	t_infoListos *aEjecutar = NULL;
	char* key;
	g_huboModificacion = 1;
	while (1) {
		if (g_huboModificacion) {
			pthread_mutex_lock(&modificacion);
			g_huboModificacion = 0;
			pthread_mutex_unlock(&modificacion);
			if (aEjecutar != NULL) {
				aEjecutar->estAnterior = calcularProximaRafaga(
						aEjecutar->estAnterior, aEjecutar->realAnterior);
				aEjecutar->realAnterior = cont;
				dictionary_put(g_listos, key, aEjecutar);
			}
			cont = 0;
			pthread_mutex_lock(&mutexListo);
			pthread_cond_wait(&ESIentrada, &mutexListo);
			key = calcularSiguienteSJF();
			aEjecutar = dictionary_remove(g_listos, key);
			pthread_mutex_unlock(&mutexListo);
			g_socketEnEjecucion = aEjecutar->socketESI;
		}
		pthread_mutex_lock(&mutexConsola);
		enviarSolicitudEjecucion(g_socketEnEjecucion);
		pthread_mutex_unlock(&mutexConsola);
		gestionarSolicitudes(g_socketEnEjecucion, (void*) gestionarRespuestaESI,
				g_logger);
		if (!g_termino) {
			gestionarSolicitudes(g_socketCoordinador,
					(void*) gestionarRespuestaCoordinador, g_logger);
			cont++;
		}
		if (g_bloqueo) {
			g_bloqueo = 0;
			bloquear(aEjecutar, cont, key);
		}
		if (g_termino) {
			g_termino = 0;
			free(aEjecutar);
		}
		free(key);
	}
}

void desbloquearESIs(t_infoBloqueo* nodo) {
	dictionary_put(g_listos, nodo->idESI, nodo->data);
}

void gestionarRespuestaCoordinador(t_paquete* unPaquete, int* socket) {
	t_list* aux;
	switch (unPaquete->codigoOperacion) {
	;
case GET:
	g_claveGET = recibirGet(unPaquete);
	g_bloqueo = 1;
	break;
case STORE:
	pthread_mutex_lock(&mutexBloqueo);
	aux = dictionary_remove(g_bloq, recibirStore(unPaquete));
	list_iterate(aux, (void*) desbloquearESIs);
	list_destroy(aux);
	pthread_mutex_unlock(&mutexBloqueo);
	pthread_mutex_lock(&modificacion);
	g_huboModificacion = 1;
	pthread_mutex_unlock(&modificacion);
	break;
case ABORTO:
	g_termino = 1;
	enviarRespuesta(ABORTO, g_socketEnEjecucion);
	break;
	}
	destruirPaquete(unPaquete);
}

void gestionarRespuestaESI(t_paquete* unPaquete, int* socket) {
	switch (unPaquete->codigoOperacion) {
	;
case ABORTO:
	g_termino = 1;
	break;
	}
	destruirPaquete(unPaquete);
}
