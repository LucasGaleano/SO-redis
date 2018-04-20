#include "instancia.h"

int main(void) {

	instanciaConfig config = cargarConfiguracionInstancia(PATH_CONFIG);

	config_destroy(config.archivoConfig);

	return 0;
}

instanciaConfig cargarConfiguracionInstancia(char * pathConfig){

	  instanciaConfig result;
	  result.archivoConfig = config_create(pathConfig);
	  result.coordinadorIpConfig = config_get_string_value(result.archivoConfig,"COORDINADOR_IP");
	  result.coordinadorPuertoConfig = config_get_int_value(result.archivoConfig,"COORDINADOR_PUERTO");
	  result.algoritmoReemplazo = config_get_string_value(result.archivoConfig,"ALGORITMO_REEMPLAZO");
	  result.puntoMontaje = config_get_string_value(result.archivoConfig,"PUNTO_MONTAJE");
	  result.nombreInstancia = config_get_string_value(result.archivoConfig,"NOMBRE_INSTANCIA");
	  result.intervaloDump = config_get_int_value(result.archivoConfig,"INTERVALO_DUMP");

	  return result;
}


