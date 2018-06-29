#include "coordinador.h"
//TODO hacer funcion logSeguro y reemplazar logTraceSeguro

int main(void) {

	signal(SIGINT, signal_handler);
	signal(SIGUSR1, signal_handler);
	g_tablaDeInstancias = crearListaInstancias();
	g_diccionarioConexiones = crearDiccionarioConexiones();
	g_diccionarioClaves = crearDiccionarioClaves();
	g_configuracion = malloc(sizeof(t_configuraciones));
	g_respuesta = true;

	g_logger = log_create("coordinador.log", "coordinador", true,
			LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	armarConfigCoordinador(g_configuracion, config);

	sem_init(&g_mutexLog, 0, 1);
	sem_init(&g_mutex_tablas, 0, 1);
	sem_init(&g_mutex_respuesta_set, 0, 0);
	sem_init(&g_mutex_respuesta_store, 0, 0);

	iniciarServidor(g_configuracion->puertoConexion);

	return 0;
}

int iniciarServidor(char* puerto) {

	struct sockaddr_storage their_addr;
	struct addrinfo hints, *res;
	int status, sockfd;
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
	agregarConexion(g_diccionarioConexiones, "coordinador", sockfd);

	bind(sockfd, res->ai_addr, res->ai_addrlen);
	//TODO cerrar o free adrrinfo

	if (-1 == listen(sockfd, 10))
		perror("listen");
	printf("esperando conexiones en puerto %s\n", puerto);
	while (1) {
		addr_size = sizeof(their_addr);
		int* cliente_fd = malloc(sizeof(int));
		*cliente_fd = accept(sockfd, (struct sockaddr*) &their_addr,
				&addr_size);

		pthread_t pid;
		pthread_create(&pid, NULL, procesarPeticion, cliente_fd);

	}
}

void* procesarPeticion(void* cliente_fd) {

	while (1) {
		char* buffer = calloc(1000, sizeof(char));
		int recvError;
		memset(buffer, '$', 1000);

		if ((recvError = recv(*(int*) cliente_fd, buffer, 1000, 0)) <= 0) {
			procesarClienteDesconectado(*(int*) cliente_fd);
			return 0;
		}

		int desplazamiento = 0;
		printf("%s\n", buffer);
		while (buffer[desplazamiento + 1] != '$') {
			int tamanio = 0;
			memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
			desplazamiento += sizeof(int);
			void* bufferPaquete = malloc(tamanio);
			memcpy(bufferPaquete, buffer + desplazamiento, tamanio);
			desplazamiento += tamanio;
			t_paquete* unPaquete = crearPaquete(bufferPaquete);
			procesarPaquete(unPaquete, *(int*) cliente_fd);
		}
		free(buffer);
	}
}

void procesarPaquete(t_paquete* paquete, int cliente_fd) {

	switch (paquete->codigoOperacion) {

	case HANDSHAKE:

		procesarHandshake(paquete, cliente_fd);
		break;

	case ENVIAR_NOMBRE_INSTANCIA:
		;

		procesarNombreInstancia(paquete, cliente_fd);
		break;

	case ENVIAR_NOMBRE_ESI:
		;
		procesarNombreESI(paquete, cliente_fd);
		break;

	case SET:
		;
		usleep(g_configuracion->retardo * 1000);
		procesarSET(paquete, cliente_fd);
		break;

	case GET:
		;
		usleep(g_configuracion->retardo * 1000);
		procesarGET(paquete, cliente_fd);
		break;

	case STORE:
		;
		usleep(g_configuracion->retardo * 1000);
		procesarSTORE(paquete, cliente_fd);
		break;

	case RESPUESTA_SOLICITUD:
		;
		procesarRespuesta(paquete, cliente_fd);
		break;

	case ENVIAR_CLAVE_ELIMINADA:
		;
		procesarClaveEliminada(paquete, cliente_fd);
		break;

	default:
		printf("codigo no reconocido\n");
		break;
	}

}

void armarConfigCoordinador(t_configuraciones* g_configuracion,
		t_config* archivoConfig) {

	g_configuracion->puertoConexion = config_get_string_value(archivoConfig,
			"PUERTO");
	g_configuracion->algoritmoDist = config_get_string_value(archivoConfig,
			"ALGORITMO_DISTRIBUCION");
	g_configuracion->cantidadEntradas = config_get_int_value(archivoConfig,
			"CANTIDAD_ENTRADAS");
	g_configuracion->tamanioEntradas = config_get_int_value(archivoConfig,
			"TAMANIO_ENTRADA");
	g_configuracion->retardo = config_get_int_value(archivoConfig, "RETARDO");

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
		instanciaElegida = buscarInstancia(tablaDeInstancias, false, NULL,
				keyDeClave, NULL);
	}

	sem_post(&g_mutex_tablas);

	return instanciaElegida;

}

void* procesarClienteDesconectado(int cliente_fd) {

	t_conexion* clienteDesconectado = buscarConexion(g_diccionarioConexiones,
	NULL, cliente_fd);
	if (clienteDesconectado == NULL) {
		return 0;
	}
	if (string_equals_ignore_case(clienteDesconectado->nombre,
			"planificador")) {
		log_error(g_logger,
				"se desconecto %s\n\n\t\t --------ESTADO INSEGURO-------\n",
				clienteDesconectado->nombre);
		raise(SIGINT);
	} else {
		log_debug(g_logger, "se desconecto %s\n", clienteDesconectado->nombre);
		t_instancia * instanciaDesconectada = buscarInstancia(
				g_tablaDeInstancias, false, clienteDesconectado->nombre, 0,
				NULL);
		if (instanciaDesconectada != NULL) { //entonces se desconecto un esi
			instanciaDesconectada->disponible = false;
			distribuirKeys(g_tablaDeInstancias);
		}
		sacarConexion(g_diccionarioConexiones, clienteDesconectado);
	}
	return 0;
}

void procesarRespuesta(t_paquete* paquete, int cliente_fd) {

	int respuesta = recibirRespuesta(paquete);
	t_conexion* conexionCliente = buscarConexion(g_diccionarioConexiones, NULL,
			cliente_fd);
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

	case ERROR_TAMANIO_CLAVE:
		log_error(g_logger, "%s mando error de tamanio de clave",
				buscarConexion(g_diccionarioConexiones, NULL, cliente_fd));
		break;

	case SET_OK:
		if (strcmp(conexionCliente->nombre, "planificador") == 0) {
			sem_post(&g_mutex_respuesta_set);
			g_respuesta = true;
			log_debug(g_logger, "%s acepto el SET", conexionCliente->nombre);
		} else {
			log_debug(g_logger, "le llego el SET  a %s",
					conexionCliente->nombre);
		}
		break;

	case SET_DEFINITIVO_ERROR:

		break;

	case SET_ERROR:
		if (strcmp(conexionCliente->nombre, "planificador") == 0) {
			sem_post(&g_mutex_respuesta_set);
			g_respuesta = false;
			log_error(g_logger, "%s no acepto el SET", conexionCliente->nombre);
		} else {
			log_error(g_logger, "hubo un error con el SET de la %s",
					conexionCliente->nombre);
		}
		break;

	case STORE_OK:
		if (strcmp(conexionCliente->nombre, "planificador") == 0) {
			sem_post(&g_mutex_respuesta_store);
			g_respuesta = true;
			log_debug(g_logger, "%s acepto el STORE", conexionCliente->nombre);
		} else {
			log_debug(g_logger, "le llego el STORE a %s",
					conexionCliente->nombre);
		}
		break;

	case STORE_ERROR:
		if (strcmp(conexionCliente->nombre, "planificador") == 0) {
			sem_post(&g_mutex_respuesta_store);
			g_respuesta = false;
			log_error(g_logger, "%s no acepto el STORE",
					conexionCliente->nombre);
		} else {
			log_error(g_logger, "hubo un error con el STORE de la %s",
					conexionCliente->nombre);
		}
		break;
	}
}

void procesarHandshake(t_paquete* paquete, int cliente_fd) {

	switch (recibirHandshake(paquete)) {
	case PLANIFICADOR:
		log_debug(g_logger, "se conecto el planificador: %i", cliente_fd);
		agregarConexion(g_diccionarioConexiones, "planificador", cliente_fd);
		break;

	case ESI:
		log_info(g_logger, "se conecto un esi: %i", cliente_fd);
		break;

	case INSTANCIA:
		log_info(g_logger, "se conecto la instancia: %i", cliente_fd);
		break;

	}

	free(paquete);

}

void procesarGET(t_paquete* paquete, int cliente_fd) {

	char* clave = recibirGet(paquete);
	agregarClaveAlSistema(g_diccionarioClaves, clave);
	t_conexion* conexionDelPlanificador = buscarConexion(
			g_diccionarioConexiones, "planificador", 0);
	enviarGet(conexionDelPlanificador->socket, clave);
	log_debug(g_logger, "enviar GET al planificador: %i, clave: %s\n",
			conexionDelPlanificador->socket, clave);
	free(clave);
	free(paquete);
}

void procesarSET(t_paquete* paquete, int cliente_fd) {

	t_claveValor* sentencia = recibirSet(paquete);
	t_conexion* conexionDelPlanificador = buscarConexion(
			g_diccionarioConexiones, "planificador", 0);

	if (!existeClaveEnSistema(g_diccionarioClaves, sentencia->clave)) {
		log_error(g_logger, "SET - la clave %s no existe.", sentencia->clave);
		enviarRespuesta(conexionDelPlanificador->socket,
							ERROR_CLAVE_NO_IDENTIFICADA);
	} else {

		log_debug(g_logger, "preguntar por SET al planificador");
		enviarSet(conexionDelPlanificador->socket, sentencia->clave,
				sentencia->valor);
		sem_wait(&g_mutex_respuesta_set); //espera respuesta set

		t_instancia* instanciaElegida = PlanificarInstancia(
				g_configuracion->algoritmoDist, sentencia->clave,
				g_tablaDeInstancias);

		if (g_respuesta == true) {

			if (instanciaElegida == NULL) {
				log_error(g_logger,
						"no se pudo planificar una instancia para la clave: %s",
						sentencia->clave);
				raise(SIGINT);
			}

			g_tiempoPorEjecucion = g_tiempoPorEjecucion + 1;

			t_conexion* conexionDeInstancia = buscarConexion(
					g_diccionarioConexiones, instanciaElegida->nombre, 0);
			if (!instanciaContieneClave(instanciaElegida->claves,
					sentencia->clave))
				agregarClaveDeInstancia(instanciaElegida, sentencia->clave);

			instanciaElegida->ultimaModificacion = g_tiempoPorEjecucion;

			logTraceSeguro(g_logger, g_mutexLog, "enviando SET %s %s a %s",
					sentencia->clave, sentencia->valor,
					instanciaElegida->nombre);

			enviarSet(conexionDeInstancia->socket, sentencia->clave,
					sentencia->valor);
			//TODO si no se puede acceder a la instancia, se le avisa al planificador
		}

		free(sentencia->clave);
		free(sentencia->valor);
		free(sentencia);
		free(paquete);
	}
}
void procesarSTORE(t_paquete* paquete, int cliente_fd) {

	char* clave = recibirStore(paquete);
	t_conexion* conexionDelPlanificador = buscarConexion(
			g_diccionarioConexiones, "planificador", 0);

	log_debug(g_logger, "preguntar por STORE al planificador");
	enviarStore(conexionDelPlanificador->socket, clave);
	sem_wait(&g_mutex_respuesta_store); //espera respuesta store

	if (g_respuesta == true) {
		mostrarTablaInstancia(g_tablaDeInstancias);
		t_instancia* instanciaElegida = buscarInstancia(g_tablaDeInstancias, false, NULL, 0, clave);
		if(instanciaElegida == NULL){ //si no esta busca en las desconectadas
			instanciaElegida = buscarInstancia(g_tablaDeInstancias, true, NULL, 0, clave);
		}
		if (existeClaveEnSistema(g_diccionarioClaves, clave)) {
			if (instanciaElegida != NULL) {
				if (instanciaElegida->disponible == true) {
					//mostrarInstancia(instanciaElegida);
					g_tiempoPorEjecucion = g_tiempoPorEjecucion + 1;

					t_conexion* conexionDeInstancia = buscarConexion(
							g_diccionarioConexiones, instanciaElegida->nombre,
							0);
					enviarStore(conexionDeInstancia->socket, clave);
					logTraceSeguro(g_logger, g_mutexLog,
							"ENVIAR STORE %s a %s\n", clave,
							instanciaElegida->nombre);

				} else {
					log_error(g_logger,
							"la clave %s se encuentra en la %s pero esta desconectada.",
							clave, instanciaElegida->nombre);
					enviarRespuesta(conexionDelPlanificador->socket,
							ERROR_CLAVE_INACCESIBLE);
				}
			} else {
				log_error(g_logger,
						"la clave %s no se encuentra en ninguna instancia.",
						clave); //no hacer nada
			}
		} else {
			log_error(g_logger, "STORE - la clave %s no existe.", clave);
			enviarRespuesta(conexionDelPlanificador->socket,
					ERROR_CLAVE_NO_IDENTIFICADA);
		}
	}

	free(paquete);
}

void procesarNombreInstancia(t_paquete* paquete, int cliente_fd) {

	char* nombre = recibirNombreInstancia(paquete);
	t_instancia* instanciaNueva = buscarInstancia(g_tablaDeInstancias, true,
			nombre, 0, NULL);

	if (instanciaNueva == NULL) {
		instanciaNueva = crearInstancia(nombre,
				g_configuracion->cantidadEntradas,
				g_configuracion->tamanioEntradas);
		agregarInstancia(g_tablaDeInstancias, instanciaNueva);
	} else {
		instanciaNueva->disponible = true;
	}

	agregarConexion(g_diccionarioConexiones, instanciaNueva->nombre,
			cliente_fd);
	distribuirKeys(g_tablaDeInstancias);
	enviarInfoInstancia(cliente_fd, g_configuracion->cantidadEntradas,
			g_configuracion->tamanioEntradas, instanciaNueva->claves);
	logTraceSeguro(g_logger, g_mutexLog, "se conecto instancia: %s", nombre);
	free(paquete);
}

//TODO probar esta funcion
void procesarClaveEliminada(t_paquete* paquete, int cliente_fd) {

	char* clave = recibirClaveEliminada(paquete);
	t_conexion* conexionDeInstancia = buscarConexion(g_diccionarioConexiones,
	NULL, cliente_fd);
	t_instancia* instanciaElegida = buscarInstancia(g_tablaDeInstancias,
	false, conexionDeInstancia->nombre, 0, NULL);
	eliminiarClaveDeInstancia(instanciaElegida, clave);

}

void procesarNombreESI(t_paquete* paquete, int cliente_fd) {

	char* nombreESI = recibirNombreEsi(paquete);

	agregarConexion(g_diccionarioConexiones, nombreESI, cliente_fd);
	logTraceSeguro(g_logger, g_mutexLog, "se conecto esi: %s", nombreESI);

	free(paquete);
}

void logTraceSeguro(t_log* logger, sem_t mutexLog, char* format, ...) {

	va_list ap;
	va_start(ap, format);
	char* mensaje = string_from_vformat(format, ap);
	sem_wait(&mutexLog);
	log_trace(logger, mensaje);
	sem_post(&mutexLog);
}

void signal_handler(int signum) {

	if (signum == SIGINT) {
		log_error(g_logger, "Cerrando conexiones\n");
		cerrarTodasLasConexiones(g_diccionarioConexiones);
		sem_destroy(&g_mutexLog);
		sem_destroy(&g_mutex_tablas);
		sem_destroy(&g_mutex_respuesta_set);
		sem_destroy(&g_mutex_respuesta_store);
		exit(0);
	}
	if (signum == SIGUSR1) {
		t_config* config = config_create(PATH_CONFIG);
		armarConfigCoordinador(g_configuracion, config);
		printf("algortimo planificaion actual: %s\n\n",
				g_configuracion->algoritmoDist);
		printf("INSTANCIAS----------------------\n");
		mostrarTablaInstancia(g_tablaDeInstancias);
		printf("CONEXIONES----------------------\n");
		mostrarDiccionario(g_diccionarioConexiones);
	}
}
