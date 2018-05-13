#include "instancia.h"

int main(void) {

	//Creo archivo de log
	logInstancia = log_create("log_Instancia.log", "instancia", true,
			LOG_LEVEL_TRACE);
	log_trace(logInstancia, "Inicio el proceso instancia \n");

	//Conecto instancia con coordinador
	conectarInstancia();


	return 0;
}

/*-------------------------Conexion-------------------------*/
void conectarInstancia() {
	//Leo la configuracion del esi
	t_config* configInstancia = leerConfiguracion();

	//Setteo las variables de configuracion
	char * coordinadorIP = config_get_string_value(configInstancia,
			"COORDINADOR_IP");
	int coordinadorPuerto = config_get_int_value(configInstancia,
			"COORDINADOR_PUERTO");
	char * algoritmoReemplazo = config_get_string_value(configInstancia,
			"ALGORITMO_REEMPLAZO");
	char * puntoMontaje = config_get_string_value(configInstancia,
			"PUNTO_MONTAJE");
	char * nombreInstancia = config_get_string_value(configInstancia,
			"NOMBRE_INSTANCIA");
	int intervaloDump = config_get_int_value(configInstancia,
			"INTERVALO_DUMP");

	//Conecto al coordinador
	socketCoordinador = conectarCliente(coordinadorIP, coordinadorPuerto,
			INSTANCIA);

	//Destruyo la configuracion
	config_destroy(configInstancia);
}

t_config* leerConfiguracion() {
	t_config* configInstancia = config_create(RUTA_CONFIG);
	if (!configInstancia) {
		log_error(logInstancia, "Error al abrir la configuracion \n");
		exit(EXIT_FAILURE);
	}
	return configInstancia;
}

