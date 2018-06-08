#include "planificador.h"

int main(void) {

	g_listos = dictionary_create();
	g_bloq = dictionary_create();
	g_clavesBloqueadas = dictionary_create();

	g_con = config_create(RUTA_CONFIGURACION_PLANIF);
	g_logger = log_create("", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(g_con, "PUERTO");

	g_est = config_get_double_value(g_con, "ESTIMACION_INICIAL");
	char* ip = config_get_string_value(g_con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(g_con, "COORDINADOR_PUERTO");
	asignarBloquedas(config_get_array_value(g_con, "CLAVES_BLOQUEADAS"));
	char* algoritmo = config_get_string_value(g_con, "ALGORITMO");
	g_alfa = (config_get_int_value(g_con, "ALFA")/100);

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
	t_respuestaStatus* respuestaStatus;
	switch (unPaquete->codigoOperacion) {
	case HANDSHAKE:
		recibirHandshakePlanif(unPaquete, socketCliente);
		break;
	case ENVIAR_NOMBRE_ESI:
		dat = malloc(sizeof(t_infoListos));
		dat->estAnterior = g_est;
		dat->realAnterior = g_est;
		dat->socketESI = *socketCliente;
		dat->tEnEspera = 0;
		pthread_mutex_lock(&mutexListo);
		dictionary_put(g_listos, recibirNombreEsi(unPaquete), dat);
		pthread_mutex_unlock(&mutexListo);
		pthread_cond_signal(&ESIentrada);
		pthread_mutex_lock(&modificacion);
		g_huboModificacion = 1;
		pthread_mutex_unlock(&modificacion);

		break;

		case RESPUESTA_STATUS:
			 respuestaStatus = recibirRespuestaStatus(unPaquete);
			 mostrarPorConsola(respuestaStatus);

	}
	destruirPaquete(unPaquete);
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
