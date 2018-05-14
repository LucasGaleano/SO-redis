#include "algoritmos.h"

inline double calcularProximaRafaga(double estimadoAnterior,
		double realAnterior) {
	return estimadoAnterior * 0.5 + realAnterior * 0.5;
}

inline double calcularRR(double tEnEspera, double estimadoAnterior,
		double realAnterior) {
	return (1
			+ tEnEspera / calcularProximaRafaga(estimadoAnterior, realAnterior));
}

inline char* asignarID(int val, char* ret) {
	char num[2] = { '\0' };
	sprintf(num, "%d", val);
	strcpy(ret, "ESI");
	return strcat(ret, num);
}

char* calcularSiguienteSJF(void) {
	t_infoListos *actual;
	double menor;

	int i = 0;
	char* auxKey = calloc(5, sizeof(char));
	auxKey = asignarID(i, auxKey);
	char* keyMenor = calloc(5, sizeof(char));
	actual = dictionary_get(g_listos, auxKey);
	menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
	strcpy(keyMenor, auxKey);
	for (i++; i < g_listos->elements_amount; i++) {
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		if (menor
				> calcularProximaRafaga(actual->estAnterior,
						actual->realAnterior)) {
			menor = calcularProximaRafaga(actual->estAnterior,
					actual->realAnterior);
			strcpy(keyMenor, auxKey);
		}
	}

	return keyMenor;
}

char* calcularSiguienteHRRN(void) {
	t_infoListos *actual;
	double mayor;

	int i = 0;
	char* auxKey = calloc(5, sizeof(char));
	auxKey = asignarID(i, auxKey);
	char* keyMenor = calloc(5, sizeof(char));
	actual = dictionary_get(g_listos, auxKey);
	mayor = calcularRR(actual->tEnEspera, actual->estAnterior,
			actual->realAnterior);
	strcpy(keyMenor, auxKey);
	for (i++; i < g_listos->elements_amount; i++) {
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		if (mayor
				< calcularProximaRafaga(actual->tEnEspera, actual->estAnterior,
						actual->realAnterior)) {
			mayor = calcularProximaRafaga(actual->tEnEspera,
					actual->estAnterior, actual->realAnterior);
			strcpy(keyMenor, auxKey);
		}
	}

	return keyMenor;
}

void envejecer(char* key, t_infoListos* data) {
	data->tEnEspera += g_tEjecucion;
}

void planificarSinDesalojo(char* algoritmo) {
	g_termino = 0;
	int cont = 0;
	t_infoListos *aEjecutar;
	char* key;

	if (strcmp(algoritmo, "SJF-SD") == 0)
		key = calcularSiguienteSJF();
	if (strcmp(algoritmo, "HRRN") == 0)
		key = calcularSiguienteHRRN();

	aEjecutar = dictionary_remove(g_listos, key);
	g_socketEnEjecucion = aEjecutar->socketESI;
	while (!g_termino && !g_bloqueo) {
		pthread_mutex_lock(&mutexConsola);
		enviarSolicitudEjecucion(g_socketEnEjecucion);
		gestionarSolicitudes(g_socketEnEjecucion, (void*) gestionarRespuestaESI,
				g_logger);
		if (!g_termino) {
			gestionarSolicitudes(g_socketCoordinador,
					(void*) gestionarRespuestaCoordinador, g_logger);
			cont++;
		}
		pthread_mutex_unlock(&mutexConsola);
	}
	if (g_bloqueo) {
		g_bloqueo = 0;
		aEjecutar->estAnterior = calcularProximaRafaga(aEjecutar->estAnterior,
				aEjecutar->realAnterior);
		aEjecutar->realAnterior = cont;
		t_infoBloqueo insert = malloc(sizeof(t_infoBloqueo));
		insert->idESI = strdup(key);
		insert->data = aEjecutar;
		if (dictionary_has_key(g_bloq, g_claveGET)) {
			list_add(dictionary_get(g_bloq, g_claveGET), insert);
		} else {
			t_list aux = list_create();
			list_add(aux, insert);
			dictionary_put(g_bloq, g_claveGET, aux);
		}
	}
	if (g_termino) {
		g_termino = 0;
		free(aEjecutar);
	}
	g_tEjecucion = cont;
	if (strcmp(algoritmo, "HRRN") == 0)
		dictionary_iterator(g_listos, (void*) envejecer);
	free(key);
}

void desbloquearESIs(t_infoBloqueo* nodo) {
	dictionary_put(g_listos, nodo->idESI, nodo->data);
}

void gestionarRespuestaCoordinador(t_paquete* unPaquete, int* socket) {
	t_list aux;
	switch (unPaquete->codigoOperacion) {
	;
case GET:
	g_claveGET = recibirGET(unPaquete);
	g_bloqueo = 1;
	break;
case STORE:
	aux = dictionary_remove(g_bloq, recibirStore(unPaquete));
	list_iterate(aux, (void*) desbloquearESIs);
	list_destroy(aux);
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
	case ABORTO:
		g_termino = 1;
		break;
	}
}
