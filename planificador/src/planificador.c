#include "planificador.h"

int g_intervaloReconexion;

int main(void) {

	g_con = config_create(RUTA_CONFIGURACION_PLANIF);
	char* ip = config_get_string_value(g_con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(g_con, "COORDINADOR_PUERTO");
	g_socketCoordinador = conectarCliente(ip, puertoCoordinador, PLANIFICADOR);
	//TODO preguntar si se pudo conectar. loguear y abortar si no se pudo

	g_listos = dictionary_create();
	g_bloq = dictionary_create();
	g_clavesTomadas = dictionary_create();

	g_logger = log_create("log.log", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(g_con, "PUERTO");

	g_keyMaxima = 0;

	g_est = config_get_double_value(g_con, "ESTIMACION_INICIAL");
	//asignarBloquedas(config_get_array_value(g_con, "CLAVES_BLOQUEADAS"));
	char* algoritmo = config_get_string_value(g_con, "ALGORITMO_PLANIFICACION");
	g_alfa = (config_get_int_value(g_con, "ALFA") / 100);
	g_intervaloReconexion = config_get_int_value(g_con, "INTERVALO_RECONEXION");

	pthread_mutex_init(&mutexBloqueo, NULL);
	pthread_mutex_init(&mutexConsola, NULL);
	pthread_mutex_init(&mutexListo, NULL);
	pthread_mutex_init(&modificacion, NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexClavesTomadas, NULL);

	sem_init(&ESIentrada, 0, 0);
	sem_init(&continua, 0, 0);

	pthread_create(&hiloAlgoritmos, NULL, (void*) planificar, algoritmo);
	pthread_create(&hiloCoordinador, NULL, (void*) atenderCoordinador, NULL);
	pthread_create(&hiloServidor, NULL, (void*) iniciarServidor,
			(void*) &puertoLocal);

	log_debug(g_logger, "inicio consola");
	//iniciarConsola();

	pthread_join(hiloCoordinador, NULL);

	liberarTodo();

	return EXIT_SUCCESS;
}

void asignarBloquedas(char** codigos) {
	int i = 0;
	while (codigos[i] != NULL) {
		t_list* ins = list_create();
		dictionary_put(g_bloq, codigos[i], ins);
	}
}

void procesarPaqueteESIs(t_paquete* unPaquete, int* socketCliente) {
	t_infoListos *dat;
	char* keyAux;
	pthread_mutex_lock(&mutexLog);
	log_debug(g_logger, "Me ha llegado una solicitud");
	pthread_mutex_unlock(&mutexLog);
	switch (unPaquete->codigoOperacion) {
	case HANDSHAKE:
		recibirHandshakePlanif(unPaquete, socketCliente);
		break;
	case ENVIAR_NOMBRE_ESI:
		if (g_keyMaxima < *socketCliente)
			g_keyMaxima = *socketCliente;
		dat = malloc(sizeof(t_infoListos));
		dat->estAnterior = g_est;
		dat->realAnterior = 0;
		dat->socketESI = *socketCliente;
		dat->tEnEspera = 0;
		dat->nombreESI = recibirNombreEsi(unPaquete);
		keyAux = string_itoa(*socketCliente);
		pthread_mutex_lock(&mutexListo);
		dictionary_put(g_listos, keyAux, dat);
		pthread_mutex_unlock(&mutexListo);
		free(keyAux);
		sem_post(&ESIentrada);
		pthread_mutex_lock(&mutexLog);
		log_info(g_logger, "Se conecto exitosamente el %s",
				recibirNombreEsi(unPaquete));
		pthread_mutex_unlock(&mutexLog);
		pthread_mutex_lock(&modificacion);
		g_huboModificacion = 1;
		pthread_mutex_unlock(&modificacion);
		break;
	case TERMINO_ESI:
		pthread_mutex_lock(&mutexLog);
		log_debug(g_logger, "%s me avisa que finalizo", g_nombreESIactual);
		pthread_mutex_unlock(&mutexLog);
		g_termino = 1;
		break;
	case ENVIAR_ERROR:
		keyAux = string_itoa(*socketCliente);
		liberarClaves(keyAux);
		free(keyAux);
		break;
	}
	destruirPaquete(unPaquete);
}

void procesarPaqueteCoordinador(t_paquete* unPaquete, int* socketCliente) {
	t_list* aux;
	switch (unPaquete->codigoOperacion) {
	case SET:
		pthread_mutex_lock(&mutexLog);
		log_debug(g_logger, "Me ha llegado un SET");
		pthread_mutex_unlock(&mutexLog);
		sem_post(&continua);
		break;
	case GET:
		pthread_mutex_lock(&mutexLog);
		log_debug(g_logger, "Me ha llegado un GET");
		pthread_mutex_unlock(&mutexLog);
		g_claveTomada = 0;
		g_claveGET = recibirGet(unPaquete);
		pthread_mutex_lock(&mutexClavesTomadas);
		dictionary_iterator(g_clavesTomadas, (void*) claveEstaTomada);
		if (g_claveTomada) {
			g_bloqueo = 1;
		} else {
			if (dictionary_has_key(g_clavesTomadas, g_idESIactual)) {
				list_add(dictionary_get(g_clavesTomadas, g_idESIactual),
						g_claveGET);
			} else {
				aux = list_create();
				list_add(aux, g_claveGET);
				dictionary_put(g_clavesTomadas, g_idESIactual, aux);
			}
			pthread_mutex_unlock(&mutexClavesTomadas);
			pthread_mutex_lock(&mutexLog);
			log_trace(g_logger, "%s ha tomado la clave %s exitosamente",
					g_nombreESIactual, g_claveGET);
			pthread_mutex_unlock(&mutexLog);
		}
		sem_post(&continua);
		break;
	case STORE:
		if (dictionary_has_key(g_bloq, recibirStore(unPaquete))) {
			pthread_mutex_lock(&mutexBloqueo);
			aux = dictionary_remove(g_bloq, recibirStore(unPaquete));
			list_iterate(aux, (void*) desbloquearESIs);
			list_destroy(aux);
			pthread_mutex_unlock(&mutexBloqueo);
			pthread_mutex_lock(&mutexLog);
			log_trace(g_logger, "%s ha liberado la clave %s exitosamente",
					g_nombreESIactual, recibirStore(unPaquete));
			pthread_mutex_unlock(&mutexLog);
		} else {
			g_termino = 1;
			enviarRespuesta(ABORTO, g_socketEnEjecucion);
			liberarClaves(g_idESIactual);
			log_error(g_logger, "%s se aborta por STORE sobre clave no tomada",
					g_nombreESIactual);
			pthread_mutex_unlock(&mutexLog);
		}
		sem_post(&continua);
		break;
	case RESPUESTA_SOLICITUD:
		g_termino = 1;
		enviarRespuesta(ABORTO, g_socketEnEjecucion);
		liberarClaves(g_idESIactual);
		sem_post(&continua);
		break;
	case RESPUESTA_STATUS:
		//mostrarPorConsola(recibirRespuestaStatus(unPaquete));
		break;
	case ENVIAR_ERROR:
		log_error(g_logger,
				"El coordinador se ha desconectado. Se aborta el planificador y sus ESIs");
		pthread_mutex_unlock(&mutexLog);
		exit(EXIT_FAILURE);
		break;
	}
	destruirPaquete(unPaquete);
}

void atenderCoordinador(void* arg) {
	while (1) {
		gestionarSolicitudes(g_socketCoordinador,
				(void*) procesarPaqueteCoordinador, g_logger);
	}
}

void liberarClaves(char* clave) {
	if (dictionary_has_key(g_clavesTomadas, clave))
		list_destroy_and_destroy_elements(
				dictionary_remove(g_clavesTomadas, clave), (void*) free);
}

void desbloquearESIs(t_infoBloqueo* nodo) {
	dictionary_put(g_listos, nodo->idESI, nodo->data);
	sem_post(&ESIentrada);
}

int condicionDeTomada(char* nodo) {
	if (strcmp(nodo, g_claveGET) == 0)
		return 1;
	return 0;
}

void claveEstaTomada(char* key, t_list* value) {
	if (!g_claveTomada && strcmp(g_idESIactual, key) != 0)
		g_claveTomada = list_any_satisfy(value, (void*) condicionDeTomada);
}

void recibirHandshakePlanif(t_paquete* unPaquete, int* socketCliente) {
	int tipoCliente;
	memcpy(&tipoCliente, unPaquete->buffer->data, sizeof(int));
	switch (tipoCliente) {
	case ESI:
		break;
	default:
		*socketCliente = -1;
	}
}

void iniciarServidor(void* puerto) {
	iniciarServer(*(int*) puerto, (void*) procesarPaqueteESIs, g_logger);
}

void planificar(char* algoritmo) {
	if (strcmp(algoritmo, "SJF-SD") == 0 || strcmp(algoritmo, "HRRN") == 0)
		planificarSinDesalojo(algoritmo);
	else
		planificarConDesalojo();
}
