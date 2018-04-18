#include "esi.h"

int main(void) {

	//Creo archivo de log
	t_log * logESI = log_create("log_ESI.log", "esi", true,
			LOG_LEVEL_TRACE);
	log_trace(logESI, "Inicio el proceso esi \n");

	//Termina esi
	log_info(logESI, "Termino el proceso esi \n");

	//Destruo archivo de log
	log_destroy(logESI);

	return EXIT_SUCCESS;
}
