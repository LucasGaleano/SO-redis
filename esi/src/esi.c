#include "esi.h"

int main(void) {

	//Creo archivo de log
	logESI = log_create("log_ESI.log", "esi", true, LOG_LEVEL_TRACE);
	log_trace(logESI, "Inicio el proceso esi \n");

	//Conecto esi con planificador y coordinador
	conectarEsi();

	//Quedo a la espera de solicitudes
	recibirSolicitudes = true;
	while (recibirSolicitudes) {
		gestionarSolicitudes(socketPlanificador, (void*) procesarPaquete,
				logESI);
	}

	//Termina esi
	log_info(logESI, "Termino el proceso esi \n");

	//Destruyo archivo de log
	log_destroy(logESI);

	return EXIT_SUCCESS;
}

/*-------------------------Conexion-------------------------*/

void conectarEsi() {
	//Leo la configuracion del esi
	t_config* configEsi = leerConfiguracion();

	//Setteo las variables de configuracion
	char * coordinadorIP = config_get_string_value(configEsi, "COORDINADOR_IP");
	int coordinadorPuerto = config_get_int_value(configEsi,
			"COORDINADOR_PUERTO");
	char * planificadorIP = config_get_string_value(configEsi,
			"PLANIFICADOR_IP");
	int planificadorPuerto = config_get_int_value(configEsi,
			"PLANIFICADOR_PUERTO");

	//Conecto al coordinador
	socketCoordinador = conectarCliente(coordinadorIP, coordinadorPuerto, ESI);

	//Conecto al planificador
	socketPlanificador = conectarCliente(planificadorIP, planificadorPuerto,
			ESI);

	//Destruyo la configuracion
	config_destroy(configEsi);
}

t_config* leerConfiguracion() {
	t_config* configEsi = config_create(RUTA_CONFIG);
	if (!configEsi) {
		log_error(logESI, "Error al abrir la configuracion \n");
		exit(EXIT_FAILURE);
	}
	return configEsi;
}

/*-------------------------Procesamiento paquetes-------------------------*/
void procesarPaquete(t_paquete * unPaquete, int * client_socket) {
	switch (unPaquete->codigoOperacion) {
	case HANDSHAKE:
		break;
	default:
		break;
	}
	destruirPaquete(unPaquete);
}
