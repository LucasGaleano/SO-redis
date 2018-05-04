#include "planificador.h"

double g_est;

int main(void) {

	g_listos = dictionary_create();
	g_bloq = dictionary_create();

	g_clavesBloqueadas = list_create();

	g_con = config_create(RUTA_CONFIGURACION_PLANIF);
	g_logger = log_create("", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(g_con, "PUERTO");
	//hablar tipo del algoritmo de planificacion
	g_est = config_get_double_value(g_con, "ESTIMACION_INICIAL");
	char* ip = config_get_string_value(g_con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(g_con, "COORDINADOR_PUERTO");
	asignarBloquedas(config_get_array_value(g_con, "CLAVES_BLOQUEADAS"));

	pthread_t* hiloServer;
	pthread_t* hiloAlgoritmos;

	pthread_create(hiloServer, NULL, (void*) iniciarServidor, puertoLocal);
	//iniciar Algoritmos en hilo

	//iniciarConsola

	pthread_join(hiloServer, NULL);

	config_destroy(g_con);
	log_destroy(g_logger);

	return EXIT_SUCCESS;
}

asignarBloquedas(char** codigos)
{
	int i = 0;
	while(codigos[i] != NULL)
	{
		t_infoClavezBloqueadas insert = malloc(sizeof(t_infoClavezBloqueadas));
		insert->codigo = strdup(codigos[i]);
		insert->consola = 1;
		list_add(g_clavesBloqueadas, insert);
	}
}

procesarPaquete(t_paquete* unPaquete, int* socketCliente)
{
	t_infoListos *dat;
	switch(unPaquete->codigoOperacion)
	{
	case HANDSHAKE:
		recibirHandshakePlanif(unPaquete, socketCliente);
		break;
	case ENVIAR_IDENTIFICACION:
		dat = malloc(sizeof(t_infoListos));
		dat->estAnterior = g_est;
		dat->realAnterior = g_est;
		dat->socketESI = *socketCliente;
		dat->tEnEspera = 0;
		dictionary_put(g_listos, recibirIdentificacion(unPaquete), dat);
	}
	destruirPaquete(unPaquete);
}

recibirHandshakePlanif(t_paquete* unPaquete,int* socketCliente)
{
	int tipoCliente;
	memcpy(&tipoCliente, unPaquete->buffer->data, sizeof(int));
	switch(tipoCliente)
	{
	case ESI:
		break;
	default:
		*socketCliente = -1;
	}
}

void iniciarServidor(int puerto)
{
	iniciarServer(puerto, (void*)procesarPaquete, g_logger);
}
