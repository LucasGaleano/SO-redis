#include "coordinador.h"





int main(void) {


	t_config* archivoConfig = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(archivoConfig);


	printf("puerto de conexion: %i\n",g_configuracion.puertoConexion);
	printf("algoritmo: %s\n",g_configuracion.algoritmoDist);
	printf("cantidad entradas: %i\n",g_configuracion.cantidadEntradas);

	config_destroy(archivoConfig);

	return 0;
}






t_configuraciones armarConfigCoordinador(t_config* archivoConfig){

	t_configuraciones configuracion;

	configuracion.puertoConexion = config_get_int_value(archivoConfig,"PUERTO");
	configuracion.algoritmoDist = config_get_string_value(archivoConfig,"ALGORITMO_DISTRIBUCION");
	configuracion.cantidadEntradas = config_get_int_value(archivoConfig,"CANTIDAD_ENTRADAS");
	configuracion.tamanioEntradas = config_get_int_value(archivoConfig,"TAMANIO_ENTRADA");
	configuracion.retardo = config_get_int_value(archivoConfig,"RETARDO");

	return configuracion;

}
