#include "esi.h"

int main(int argc, char **argv) {

	//Creo archivo de log
	logESI = log_create("log_ESI.log", "esi", true, LOG_LEVEL_TRACE);
	log_trace(logESI, "Inicio el proceso esi \n");

	//Mapeo el archivo a memoria
	size_t tamArch;
	FILE * archivofd;
	char * rutaScript = argv[1];

	archivo = abrirArchivo(rutaScript, &tamArch, &archivofd);

	//Setteo la ip para que arranque a leer desede el principio del archivo
	ip = 0;

	//Conecto esi con planificador y coordinador
	conectarEsi();

	//Quedo a la espera de solicitudes
	recibirSolicitudes = true;
	while (recibirSolicitudes) {
		gestionarSolicitudes(socketPlanificador, (void*) procesarPaquete,
				logESI);
	}

	//Cierro el archivo
	munmap(archivo, tamArch);
	fclose(archivofd);

	//Termina esi
	log_trace(logESI, "Termino el proceso esi \n");

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
	char * nombre = config_get_string_value(configEsi, "NOMBRE");

	//Conecto al coordinador
	socketCoordinador = conectarCliente(coordinadorIP, coordinadorPuerto, ESI);

	//Conecto al planificador
	socketPlanificador = conectarCliente(planificadorIP, planificadorPuerto,
			ESI);
	enviarNombreEsi(socketPlanificador, nombre);

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
	case SOLICITUD_EJECUCION:
		procesarSolicitudEjecucion();
		break;
	case ENVIAR_ERROR:
		procesarError();
		break;
	case RESPUESTA_SOLICITUD:
		procesarRespuestaSolicitud();
		break;
	default:
		break;
	}
	destruirPaquete(unPaquete);
}

void procesarSolicitudEjecucion() {
	//Busco la sentencia a ejecutar
	char * sentencia = proximaSentencia(archivo, &ip);

	//Parceo la sentencia
	t_esi_operacion parsed = parse(sentencia);

	//Verifico si el parceo es valido
	if (parsed.valido) {
		switch (parsed.keyword) {
		case GET:
			log_trace(logESI,"GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
			enviarGet(socketCoordinador, parsed.argumentos.GET.clave);
			break;
		case SET:
			log_trace(logESI,"SET\tclave: <%s>\tvalor: <%s>\n",
					parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
			enviarSet(socketCoordinador, parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
			break;
		case STORE:
			log_trace(logESI, "STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
			enviarStore(socketCoordinador, parsed.argumentos.STORE.clave);
			break;
		default:
			log_warning(logESI, "No pude interpretar <%s>\n", sentencia);
			exit(EXIT_FAILURE);
		}
		destruir_operacion(parsed);
	} else {
		log_error(logESI, "La linea <%s> no es valida\n", sentencia);
		exit(EXIT_FAILURE);
	}

	//Libero memoria
	free(sentencia);
}

void procesarError(){
	recibirSolicitudes = false;
	log_error(logESI, "Se desconecto el servidor");
}

void procesarRespuestaSolicitud(){
	recibirSolicitudes = false;
	log_error(logESI, "Fallo critico");
}

/*-------------------------Funciones auxiliares-------------------------*/
void * abrirArchivo(char * rutaArchivo, size_t * tamArc, FILE ** archivo) {
//Abro el archivo
	*archivo = fopen(rutaArchivo, "r");

	if (*archivo == NULL) {
		log_error(logESI, "%s: No existe el archivo", rutaArchivo);
		perror("Error al abrir el archivo");
		exit(EXIT_FAILURE);
	}

//Copio informacion del archivo
	struct stat statArch;

	stat(rutaArchivo, &statArch);

//Tamaño del archivo que voy a leer
	*tamArc = statArch.st_size;

//Leo el total del archivo y lo asigno al buffer
	int fd = fileno(*archivo);
	void * dataArchivo = mmap(0, *tamArc, PROT_READ, MAP_SHARED, fd, 0);

//Confirmo la lectura del archivo
	log_debug(logESI, "Abrio el archivo: %s", rutaArchivo);

	return dataArchivo;
}

char * proximaSentencia(char * archivo, int * ip) {
	char * archivoNoLeido = archivo + (*ip);

	int i;

	for (i = 0; archivoNoLeido[i] != '\n' && string_length(archivoNoLeido) >= i;
			++i)
		;

	char * sentencia = malloc(sizeof(char) * i + 1);

	memcpy(sentencia, archivoNoLeido, i);

	*ip = *ip + i + 1;

	return sentencia;
}
