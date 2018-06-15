#include "coordinador.h"
//TODO hacer funcion logSeguro y reemplazar logTraceSeguro
//TODO porque la tabla de instancia tiene socket si esta en el diccionario.

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
	g_diccionarioConexiones = crearDiccionarioConexiones();

	g_logger = log_create("coordinador.log", "coordinador", true,
			LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(config);

	sem_init(&g_mutexLog, 0, 1);
	sem_init(&g_mutex_tablas, 0, 1);

	iniciarServidor(g_configuracion.puertoConexion);

	sem_destroy(&g_mutexLog);
	sem_destroy(&g_mutex_tablas);
	return 0;
}


void* procesarPeticion(int cliente_fd){

	while(1){
			char* buffer = calloc(1000, sizeof(char));
			int recvError;
			memset(buffer,'$',1000);

			if( (recvError = recv(cliente_fd, buffer, 1000, 0)) <= 0){
				log_error(g_logger,"se desconecto socket: %i\n",cliente_fd);
				return -1;
				}

			int desplazamiento = 0;
			printf("%s\n",buffer);
			while (buffer[desplazamiento+1] != '$') {
				int tamanio = 0;
				memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
				desplazamiento += sizeof(int);
				void* bufferPaquete = malloc(tamanio);
				memcpy(bufferPaquete, buffer + desplazamiento, tamanio);
				desplazamiento += tamanio;
				t_paquete* unPaquete = crearPaquete(bufferPaquete);
				procesarPaquete(unPaquete, cliente_fd);
			}
			free(buffer);
		}
}

void procesarPaquete(t_paquete* paquete,int cliente_fd) {

	switch (paquete->codigoOperacion) {

	case HANDSHAKE:

		procesarHandshake(paquete, cliente_fd);
		break;

	case ENVIAR_NOMBRE_ESI:
		;
		procesarNombreESI(paquete, cliente_fd);
		break;

	case ENVIAR_NOMBRE_INSTANCIA:
		;

		procesarNombreInstancia(paquete, cliente_fd);
		break;

	case SET:
		;

		procesarSET(paquete, cliente_fd);
		break;

	case GET:
		;
		procesarGET(paquete, cliente_fd);
		break;

	case STORE:
		;

		procesarSTORE(paquete, cliente_fd);
		break;

	case RESPUESTA_SOLICITUD:
		;
		procesarRespuesta(paquete, cliente_fd);
		break;

	default:
		printf("codigo no reconocido\n");
		break;
	}

}

t_configuraciones armarConfigCoordinador(t_config* archivoConfig) {

	t_configuraciones configuracion;

	configuracion.puertoConexion = config_get_string_value(archivoConfig,
			"PUERTO");
	configuracion.algoritmoDist = config_get_string_value(archivoConfig,
			"ALGORITMO_DISTRIBUCION");
	configuracion.cantidadEntradas = config_get_int_value(archivoConfig,
			"CANTIDAD_ENTRADAS");
	configuracion.tamanioEntradas = config_get_int_value(archivoConfig,
			"TAMANIO_ENTRADA");
	configuracion.retardo = config_get_int_value(archivoConfig, "RETARDO");

	return configuracion;

}

t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion, char* clave,
		t_list* tablaDeInstancias) {

	sem_wait(&g_mutex_tablas);
	t_instancia* instanciaElegida = NULL;

	if (string_equals_ignore_case(algoritmoDePlanificacion, "LSU"))
		instanciaElegida = traerInstanciaMasEspacioDisponible(
				tablaDeInstancias);

	if (string_equals_ignore_case(algoritmoDePlanificacion, "EL"))
		instanciaElegida = traerUltimaInstanciaUsada(tablaDeInstancias);

	if (string_equals_ignore_case(algoritmoDePlanificacion, "KE")) {
		int keyDeClave = (int) string_substring(clave, 0, 1);
		instanciaElegida = buscarInstancia(tablaDeInstancias, NULL, keyDeClave,
				0);
	}

	sem_post(&g_mutex_tablas);

	return instanciaElegida;

}

void* procesarRespuesta(t_paquete* paquete, int cliente_fd) {

	int respuesta = recibirRespuesta(paquete);

	switch (respuesta) {

	case ERROR_ESPACIO_INSUFICIENTE:
		/*
		 * TODO
		 * esperamos que todas las ejecuciones terminen.
		 * las bloqueamos.
		 * mandamos a hacer la compactacion a todas las instancias.
		 * a la instancia que devolvio error por espacio insuficiente le enviamos
		 * otra vez trabajo actual como SET_DEFINITIVO
		 * reanudamos.
		 *
		 * */

		break;

	case ERROR_CLAVE_NO_IDENTIFICADA:
		//TODO mandamos error a planificador
		break;

	}
	return 0;

}

void* procesarHandshake(t_paquete* paquete, int cliente_fd) {

	switch (recibirHandshake(paquete)) {
	case PLANIFICADOR:
		log_debug(g_logger, "se conecto el planificador: %i", cliente_fd);
		agregarConexion(g_diccionarioConexiones, "planificador", &cliente_fd);
		break;

	case ESI:
		log_info(g_logger, "Se conecto un ESI");
		break;

	case INSTANCIA:
		log_info(g_logger, "Se conecto una instancia");
		break;

	}

	free(paquete);
	return 0;

}

void* procesarSET(t_paquete* paquete, int cliente_fd) {

	t_claveValor* sentencia = recibirSet(paquete);
	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, sentencia->clave,
			g_tablaDeInstancias);
	mostrarInstancia(instanciaElegida);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones,
			"planificador");

	usleep(g_configuracion.retardo);

	int* socketInstancia = conseguirConexion(g_diccionarioConexiones,
			instanciaElegida->nombre);

	logTraceSeguro(g_logger, g_mutexLog, "enviando SET %s, %s a %s",
			sentencia->clave, sentencia->valor, instanciaElegida->nombre);

	list_add(instanciaElegida->claves,sentencia->clave);
	enviarSet(*socketInstancia, sentencia->clave, sentencia->valor);
	enviarSet(socketDelPlanificador, sentencia->clave, sentencia->valor);

	//TODO si no se puede acceder a la instancia, se le avisa al planificador

	free(paquete);
	return 0;
}

void* procesarGET(t_paquete* paquete, int cliente_fd) {

	char* clave = recibirGet(paquete);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones,
			"planificador");

	log_debug(g_logger, "enviar GET al planificador: %i, clave: %s\n",
			socketDelPlanificador, clave);

	enviarGet(socketDelPlanificador, clave);

	free(paquete);
	return 0;

}

void* procesarSTORE(t_paquete* paquete, int cliente_fd) {

	log_debug(g_logger, "Entro al procesarSTORE");

	char* clave = recibirStore(paquete);

	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, clave, g_tablaDeInstancias);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones,
			"planificador");

	// Al planificar busco solo instancias disponibles por lo tanto puede devolver nulo si todavia no se recargo la tabla de instancias
	if (instanciaElegida != NULL) {

		int* socketInstancia = conseguirConexion(g_diccionarioConexiones,
				instanciaElegida->nombre);

		enviarStore(*socketInstancia, clave);
		//TODO: contemplar posible error en la liberacion de la clave en la instancia , clave inaccesible instancia
		enviarStore(socketDelPlanificador, clave);

		logTraceSeguro(g_logger, g_mutexLog,
				"ENVIAR STORE planificador %i, clave: %s\n",
				socketDelPlanificador, clave);
	} else {

		enviarRespuesta(socketDelPlanificador, ERROR_CLAVE_INACCESIBLE);

	}

	free(paquete);
	return 0;
}

void* procesarNombreInstancia(t_paquete* paquete, int cliente_fd) {

	char* nombre = recibirNombreInstancia(paquete);

	t_instancia* instanciaNueva = crearInstancia(nombre);
	agregarConexion(g_diccionarioConexiones, instanciaNueva->nombre,
			&cliente_fd);
	agregarInstancia(g_tablaDeInstancias, instanciaNueva);
	distribuirKeys(g_tablaDeInstancias);
	enviarInfoInstancia(cliente_fd, g_configuracion.cantidadEntradas,
			g_configuracion.tamanioEntradas,instanciaNueva->claves);
	logTraceSeguro(g_logger, g_mutexLog, "se conecto: %s", nombre);

	free(paquete);
	return 0;
}

void* procesarNombreESI(t_paquete* paquete, int cliente_fd){

	char* nombreESI = recibirNombreEsi(paquete);

	agregarConexion(g_diccionarioConexiones, nombreESI, &cliente_fd);
	logTraceSeguro(g_logger, g_mutexLog, "se conecto: %s", nombreESI);

	free(paquete);
	return 0;
}

void logTraceSeguro(t_log* logger, sem_t mutexLog, char* format, ...) {

	va_list ap;
	va_start(ap, format);
	char* mensaje = string_from_vformat(format, ap);
	sem_wait(&mutexLog);
	log_trace(logger, mensaje);
	sem_post(&mutexLog);
}

int iniciarServidor(char* puerto) {

	struct sockaddr_storage their_addr;
	struct addrinfo hints, *res;
	int status, cliente_fd, sockfd;
	socklen_t addr_size;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((status = getaddrinfo(NULL, puerto, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


	bind(sockfd, res->ai_addr, res->ai_addrlen);
	//TODO cerrar o free adrrinfo

	if (-1 == listen(sockfd, 10))
		perror("listen");
	printf("esperando conexiones en puerto: %s\n", puerto);
	while (1) {
		fflush(stdout);
		addr_size = sizeof(their_addr);

		cliente_fd = accept(sockfd, (struct sockaddr*) &their_addr, &addr_size);

		pthread_t pid;
		pthread_create(&pid, NULL, procesarPeticion, cliente_fd);

		//close(cliente_fd);


}

}
