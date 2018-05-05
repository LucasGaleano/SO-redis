#include "planificador.h"

t_dictionary* g_listos;
t_dictionary* g_exec;
t_dictionary* g_term;
t_dictionary* g_bloq;

int main(void) {

	g_listos = dictionary_create();
	g_exec = dictionary_create();
	g_term = dictionary_create();
	g_bloq = dictionary_create();

	t_config* con = config_create(RUTA_CONFIGURACION_PLANIF);
	t_log* logger = log_create("", "Planificador", 1, LOG_LEVEL_TRACE);

	int puertoLocal = config_get_int_value(con, "PUERTO");
	//hablar tipo del algoritmo de planificacion
	double est = config_get_double_value(con, "ESTIMACION_INICIAL");
	char* ip = config_get_string_value(con, "COORDINADOR_IP");
	int puertoCoordinador = config_get_int_value(con, "COORDINADOR_PUERTO");
	//claves bloqueadas



	config_destroy(con);
	log_destroy(logger);

	return EXIT_SUCCESS;
}

