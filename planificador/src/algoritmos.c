#include "algoritmos.h"

static double calcularProximaRafaga(double estimadoAnterior,
		double realAnterior, double arg) {
	return estimadoAnterior * g_alfa + realAnterior * (1 - g_alfa);
}

static double calcularRR(double estimadoAnterior, double realAnterior,
		double tEnEspera) {
	return (1
			+ tEnEspera
					/ calcularProximaRafaga(estimadoAnterior, realAnterior, 0));
}

static int esMenor(int comp1, int comp2) {
	return comp1 < comp2;
}

static int esMayor(int comp1, int comp2) {
	return comp1 > comp2;
}

static char* asignarID(int val, char* ret) {
	char* num = string_itoa(val);
	ret = malloc(strlen(num) + 3);
	strcpy(ret, "ESI");
	return strcat(ret, num);
}

static void bloquear(t_infoListos* bloq, int nuevoReal, char* key) {
	bloq->estAnterior = calcularProximaRafaga(bloq->estAnterior,
			bloq->realAnterior, 0);
	bloq->realAnterior = nuevoReal;
	t_infoBloqueo* insert = malloc(sizeof(t_infoBloqueo));
	insert->idESI = key;
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
	log_trace(g_logger, "Se ha bloqueado %s bajo la clave %s", key, g_claveGET);
	pthread_mutex_unlock(&mutexLog);
}

static liberarSalida(void* arg) {
	free(g_claveGET);
	free(g_idESIactual);
}

static char* calcularSiguiente(double (*calculadorProx)(double, double, double),
		int (*ponderacion)(int, int)) {
	t_infoListos *actual;
	double unValor;

	int i = 0;
	char* auxKey = asignarID(i, auxKey);
	char* key;
	actual = dictionary_get(g_listos, auxKey);
	unValor = calculadorProx(actual->tEnEspera, actual->estAnterior,
			actual->realAnterior);
	key = strdup(auxKey);
	for (i++; i < dictionary_size(g_listos); i++) {
		free(auxKey);
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		double prox = calcularRR(actual->tEnEspera, actual->estAnterior,
				actual->realAnterior);
		if (ponderacion(unValor, prox)) {
			unValor = prox;
			strcpy(key, auxKey);
		}
	}
	log_trace(g_logger, "Se ejecuta %s", key);
	return key;
}

static void envejecer(char* key, t_infoListos* data) {
	data->tEnEspera += g_tEjecucion;
}

extern void planificarSinDesalojo(char* algoritmo) {
	int cont;
	t_infoListos *aEjecutar;
	char* key;
	pthread_cleanup_push((void*) liberarSalida, NULL);
	while (1) {
		cont = 0;
		sem_wait(&ESIentrada);
		pthread_mutex_lock(&mutexListo);
		if (strcmp(algoritmo, "SJF-SD") == 0)
			key = calcularSiguiente((void*) calcularProximaRafaga,
					(void*) esMayor);
		if (strcmp(algoritmo, "HRRN") == 0)
			key = calcularSiguiente((void*) calcularRR, (void*) esMenor);

		g_idESIactual = strdup(key);
		aEjecutar = dictionary_remove(g_listos, key);
		pthread_mutex_unlock(&mutexListo);
		g_socketEnEjecucion = aEjecutar->socketESI;
		while (!g_termino && !g_bloqueo) {
			pthread_mutex_lock(&mutexConsola);
			enviarSolicitudEjecucion(g_socketEnEjecucion);
			pthread_mutex_unlock(&mutexConsola);
			sem_wait(&continua);
			if (!g_termino) {
				sem_wait(&continua);
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
			log_trace(g_logger, "%s ha terminado su ejecucion", key);
		}
		if (strcmp(algoritmo, "HRRN") == 0) {
			g_tEjecucion = cont;
			pthread_mutex_lock(&mutexListo);
			dictionary_iterator(g_listos, (void*) envejecer);
			pthread_mutex_unlock(&mutexListo);
		}
	}
	pthread_cleanup_pop(1);
}

extern void planificarConDesalojo(void) {
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
						aEjecutar->estAnterior, aEjecutar->realAnterior, 0);
				aEjecutar->realAnterior = cont;
				pthread_mutex_lock(&mutexListo);
				dictionary_put(g_listos, key, aEjecutar);
				pthread_mutex_unlock(&mutexListo);
				sem_post(&ESIentrada);
				log_trace(g_logger, "Se desaloja %s para replanificar", key);
			}
			cont = 0;
			sem_wait(&ESIentrada);
			pthread_mutex_lock(&mutexListo);
			key = calcularSiguiente((void*) calcularProximaRafaga,
					(void*) esMayor);
			g_idESIactual = strdup(key);
			aEjecutar = dictionary_remove(g_listos, key);
			pthread_mutex_unlock(&mutexListo);
			g_socketEnEjecucion = aEjecutar->socketESI;
		}
		pthread_mutex_lock(&mutexConsola);
		enviarSolicitudEjecucion(g_socketEnEjecucion);
		pthread_mutex_unlock(&mutexConsola);
		sem_wait(&continua);
		if (!g_termino) {
			sem_wait(&continua);
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
	}
}
