#include "planificador.h"

double g_est;

int main(void) {

	g_listos = dictionary_create();
	g_exec = dictionary_create();
	g_term = dictionary_create();
	g_bloq = dictionary_create();

	g_con = config_create(RUTA_CONFIGURACION_PLANIF);
	g_logger = log_create("", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(g_con, "PUERTO");
	//hablar tipo del algoritmo de planificacion
	g_est = config_get_double_value(g_con, "ESTIMACION_INICIAL");
	char* ip = config_get_string_value(g_con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(g_con, "COORDINADOR_PUERTO");
	asignarBloquedas(config_get_array_value(g_con, "CLAVES_BLOQUEADAS"));

	pthread_t* hiloServer;

	pthread_create(hiloServer, NULL, (void*) iniciarServidor, puertoLocal);

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
		dictionary_put(g_bloq, codigos[i], NULL);
		i++;
	}
}

procesarPaquete(t_paquete* unPaquete, int* socketCliente)
{
	switch(unPaquete->codigoOperacion)
	{
	case HANDSHAKE:
		recibirHandshake(unPaquete, socketCliente);
		break;
	case ENVIAR_IDENTIFICACION:
		t_infoListos *dat = malloc(sizeof(t_infoListos));
		dat->estAnterior = g_est;
		dat->realAnterior = g_est;
		dat->socketESI = *socketCliente;
		dictionary_put(g_listos, recibirIdentificacion(unPaquete), dat);
	}
	destruirPaquete(unPaquete);
}

recibirHandshake(t_paquete* unPaquete,int* socketCliente)
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
