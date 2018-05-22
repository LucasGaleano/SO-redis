#include "instancia.h"

int main(void) {
	//Creo archivo de log
	logInstancia = log_create("log_Instancia.log", "instancia", true,
			LOG_LEVEL_TRACE);
	log_trace(logInstancia, "Inicio el proceso instancia \n");

	//Conecto instancia con coordinador
	conectarInstancia();

	//Quedo a la espera de solicitudes
	recibirSolicitudes = true;
	while (recibirSolicitudes) {
		gestionarSolicitudes(socketCoordinador, (void*) procesarPaquete,
				logInstancia);
	}

	//Termina esi
	log_trace(logInstancia, "Termino el proceso instancia \n");

	//Destruyo archivo de log
	log_destroy(logInstancia);

	return EXIT_SUCCESS;
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
	algoritmoReemplazo = config_get_string_value(configInstancia,
			"ALGORITMO_REEMPLAZO");
	puntoMontaje = config_get_string_value(configInstancia, "PUNTO_MONTAJE");
	char * nombreInstancia = config_get_string_value(configInstancia,
			"NOMBRE_INSTANCIA");
	intervaloDump = config_get_int_value(configInstancia, "INTERVALO_DUMP");

	//Conecto al coordinador
	socketCoordinador = conectarCliente(coordinadorIP, coordinadorPuerto,
			INSTANCIA);

	enviarNombreInstancia(socketCoordinador, nombreInstancia);

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

/*-------------------------Procesamiento paquetes-------------------------*/
void procesarPaquete(t_paquete * unPaquete, int * client_socket) {
	switch (unPaquete->codigoOperacion) {
	case ENVIAR_INFO_INSTANCIA:
		procesarEnviarInfoInstancia(unPaquete);
		break;
	default:
		break;
	}
	destruirPaquete(unPaquete);
}

void procesarEnviarInfoInstancia(t_paquete * unPaquete) {
	t_infoInstancia * info = recibirInfoInstancia(unPaquete);

	//Setteo tam de entrada y cantidad
	info->cantEntradas = cantEntradas;
	info->tamanioEntrada = tamanioEntrada;

	//Creo el espacio de almacenamiento
	storage = malloc(cantEntradas * tamanioEntrada);

	//Libero memoria
	free(info);

}

/*-------------------------Tabla de entradas-------------------------*/
void crearTablaEntradas(void) {
	tablaEntradas = list_create();
}

void destruirTabla(void){
	void eliminarEntrada(t_tabla_entradas * entrada){
		free(entrada->clave);
		if(entrada->entrada != NULL)free(entrada->entrada);
		free(entrada);
	}

	list_destroy_and_destroy_elements(tablaEntradas, (void *) eliminarEntrada);
}

t_tabla_entradas * buscarEntrada(char * clave) {
	bool esEntradaBuscada(t_tabla_entradas * entrada) {
		return string_equals_ignore_case(entrada->clave, clave);
	}

	t_tabla_entradas * registroEntrada = list_find(tablaEntradas,
			(void*) esEntradaBuscada);

	if (registroEntrada == NULL)
		return NULL;

	return registroEntrada;
}

void agregarClave(char * clave) {
	t_tabla_entradas * registroEntrada = malloc(sizeof(t_tabla_entradas));

	registroEntrada->clave = strdup(clave);

	registroEntrada->tamanio = 0;

	list_add(tablaEntradas, registroEntrada);
}

void eliminarClave(char * clave) {
	bool esEntradaBuscada(t_tabla_entradas * entrada) {
		return string_equals_ignore_case(entrada->clave, clave);
	}

	t_tabla_entradas * entradaBuscada = list_remove_by_condition(tablaEntradas,
			(void*) esEntradaBuscada);

	if (entradaBuscada != NULL) {
		//Libero memoria
		free(entradaBuscada->clave);
		if (entradaBuscada->entrada != NULL)
			free(entradaBuscada->entrada);
		free(entradaBuscada);
	}
}

void mostrarTabla(void) {
	int i;

	printf("Clave			Tamanio \n");
	printf("-------------------------------\n");

	for (i = 0; i < tablaEntradas->elements_count; i++) {
		t_tabla_entradas * entrada = list_get(tablaEntradas, i);
		printf("%s			%d \n", entrada->clave, entrada->tamanio);
	}
}

/*-------------------------Algoritmo circular -------------------------*/
//bool * bitMapEntradas[cantEntradas];
