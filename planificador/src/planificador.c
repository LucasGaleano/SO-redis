#include "planificador.h"

int main(void) {

	g_listos = dictionary_create();
	g_bloq = dictionary_create();
	g_clavesTomadas = dictionary_create();

	g_con = config_create(RUTA_CONFIGURACION_PLANIF);
	g_logger = log_create("", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(g_con, "PUERTO");

	g_est = config_get_double_value(g_con, "ESTIMACION_INICIAL");
	char* ip = config_get_string_value(g_con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(g_con, "COORDINADOR_PUERTO");
	asignarBloquedas(config_get_array_value(g_con, "CLAVES_BLOQUEADAS"));
	char* algoritmo = config_get_string_value(g_con, "ALGORITMO");
	g_alfa = (config_get_int_value(g_con, "ALFA") / 100);

	pthread_mutex_init(&mutexBloqueo, NULL);
	pthread_mutex_init(&mutexConsola, NULL);
	pthread_mutex_init(&mutexListo, NULL);
	pthread_mutex_init(&modificacion, NULL);
	sem_init(&ESIentrada, 0, 0);
	sem_init(&continua, 0, 0);

	pthread_t hiloServidor;
	pthread_t hiloAlgoritmos;

	g_socketCoordinador = conectarCliente(ip, puertoCoordinador, PLANIFICADOR);

	pthread_create(&hiloServidor, NULL, (void*) iniciarServidor, &puertoLocal);
	pthread_create(&hiloAlgoritmos, NULL, (void*) planificar, algoritmo);

	iniciarConsola();

	pthread_join(hiloServidor, NULL);

	config_destroy(g_con);
	log_destroy(g_logger);

	return EXIT_SUCCESS;
}

void asignarBloquedas(char** codigos) {
	int i = 0;
	while (codigos[i] != NULL) {
		t_list* ins = list_create();
		dictionary_put(g_bloq, codigos[i], ins);
	}
}

void procesarPaquete(t_paquete* unPaquete, int* socketCliente) {
	t_infoListos *dat;
	t_list* aux;
	switch (unPaquete->codigoOperacion) {
	case HANDSHAKE:
		recibirHandshakePlanif(unPaquete, socketCliente);
		break;
	case ENVIAR_NOMBRE_ESI:
		dat = malloc(sizeof(t_infoListos));
		dat->estAnterior = g_est;
		dat->realAnterior = 0;
		dat->socketESI = *socketCliente;
		dat->tEnEspera = 0;
		pthread_mutex_lock(&mutexListo);
		dictionary_put(g_listos, recibirNombreEsi(unPaquete), dat);
		pthread_mutex_unlock(&mutexListo);
		sem_post(&ESIentrada);
		pthread_mutex_lock(&modificacion);
		g_huboModificacion = 1;
		pthread_mutex_unlock(&modificacion);
		break;
	case GET:
		g_claveTomada = 0;
		g_claveGET = recibirGet(unPaquete);
		dictionary_iterator(g_clavesTomadas, (void*) claveEstaTomada);
		if (g_claveTomada) {
			g_bloqueo = 1;
			pthread_mutex_lock(&modificacion);
			g_huboModificacion = 1;
			pthread_mutex_unlock(&modificacion);
		} else {
			aux = list_create();
			list_add(aux, g_claveGET);
			dictionary_put(g_clavesTomadas, g_idESIactual, aux);
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
			pthread_mutex_lock(&modificacion);
			g_huboModificacion = 1;
			pthread_mutex_unlock(&modificacion);
		} else {
			g_termino = 1;
			enviarRespuesta(ABORTO, g_socketEnEjecucion);
		}
		sem_post(&continua);
		break;
	case ABORTO:
		g_termino = 1;
		enviarRespuesta(ABORTO, g_socketEnEjecucion);
		sem_post(&continua);
		break;
	case TERMINO_ESI:
		g_termino = 1;
		sem_post(&continua);
		break;
	}
	destruirPaquete(unPaquete);
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
	if (!g_claveTomada)
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

void iniciarServidor(int puerto) {
	iniciarServer(puerto, (void*) procesarPaquete, g_logger);
}

void planificar(char* algoritmo) {
	if (strcmp(algoritmo, "SJF_SD") == 0 || strcmp(algoritmo, "HRRN") == 0)
		planificarSinDesalojo(algoritmo);
	else
		planificarConDesalojo();
}
