#include "coordinador.h"
//TODO hacer funcion logSeguro y reemplazar logTraceSeguro


int main(void) {

	signal(SIGINT, planificador_handler);

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
	agregarConexion(g_diccionarioConexiones,"coordinador",sockfd);


	bind(sockfd, res->ai_addr, res->ai_addrlen);
	//TODO cerrar o free adrrinfo

	if (-1 == listen(sockfd, 10))
		perror("listen");
	printf("esperando conexiones en puerto %s\n", puerto);
	while (1) {
		addr_size = sizeof(their_addr);
		int* cliente_fd = malloc(sizeof(int));
		*cliente_fd = accept(sockfd, (struct sockaddr*) &their_addr, &addr_size);

		pthread_t pid;
		pthread_create(&pid, NULL, procesarPeticion, cliente_fd);

	}
}

void* procesarPeticion(int* cliente_fd){

	while(1){
			char* buffer = calloc(1000, sizeof(char));
			int recvError;
			memset(buffer,'$',1000);


			if( (recvError = recv(*cliente_fd, buffer, 1000, 0)) <= 0){
				procesarClienteDesconectado(*cliente_fd);
				return 0;
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
				procesarPaquete(unPaquete, *cliente_fd);
			}
			free(buffer);
		}
}

void procesarPaquete(t_paquete* paquete,int cliente_fd) {



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

	case ENVIAR_CLAVE_ELIMINADA:
		;
		procesarClaveEliminada(paquete, cliente_fd);
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
		instanciaElegida = buscarInstancia(tablaDeInstancias,false, NULL, keyDeClave);
	}

	sem_post(&g_mutex_tablas);

	return instanciaElegida;

}

void procesarClienteDesconectado(int cliente_fd){

	t_conexion* clienteDesconectado = buscarConexion(g_diccionarioConexiones,NULL,cliente_fd);
	if(strcmp(clienteDesconectado->nombre,"planificador") == 0){
		log_error(g_logger,"se desconecto %s\n\n\t\t --------ESTADO INSEGURO-------\n",clienteDesconectado->nombre);
		raise(SIGINT);
	}
	else{
		log_debug(g_logger,"se desconecto %s\n",clienteDesconectado->nombre);
		t_instancia * instanciaDesconectada = buscarInstancia( g_tablaDeInstancias,false,clienteDesconectado, 0);
		instanciaDesconectada->disponible = false;
		distribuirKeys(g_tablaDeInstancias);
		sacarConexion(g_diccionarioConexiones,clienteDesconectado);
	}
}

void procesarRespuesta(t_paquete* paquete, int cliente_fd) {

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

void procesarSET(t_paquete* paquete, int cliente_fd) {

	t_claveValor* sentencia = recibirSet(paquete);
	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, sentencia->clave,
			g_tablaDeInstancias);

	list_add(instanciaElegida->claves,sentencia->clave);

	g_tiempoPorEjecucion = g_tiempoPorEjecucion + 1;


	t_conexion* conexionDelPlanificador = buscarConexion(g_diccionarioConexiones,"planificador",0);
	usleep(g_configuracion.retardo*1000);

	t_conexion* conexionDeInstancia = BuscarConexion(g_diccionarioConexiones, instanciaElegida->nombre, 0);

	logTraceSeguro(g_logger, g_mutexLog, "enviando SET %s, %s a %s",
			sentencia->clave, sentencia->valor, instanciaElegida->nombre);

	list_add(instanciaElegida->claves,sentencia->clave);
	enviarSet(conexionDeInstancia->socket, sentencia->clave, sentencia->valor);
	enviarSet(conexionDelPlanificador->socket, sentencia->clave, sentencia->valor);

	//TODO si no se puede acceder a la instancia, se le avisa al planificador

	free(paquete);
}

void procesarGET(t_paquete* paquete, int cliente_fd) {

	char* clave = recibirGet(paquete);

	t_conexion* conexionDelPlanificador = buscarConexion(g_diccionarioConexiones,"planificador",0);

	log_debug(g_logger, "enviar GET al planificador: %i, clave: %s\n",
			conexionDelPlanificador->socket, clave);

	enviarGet(conexionDelPlanificador->socket, clave);

	free(paquete);

}

void procesarSTORE(t_paquete* paquete, int cliente_fd) {

	log_debug(g_logger, "Entro al procesarSTORE");

	char* clave = recibirStore(paquete);

	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, clave, g_tablaDeInstancias);

	t_conexion* conexionDelPlanificador = buscarConexion(g_diccionarioConexiones,"planificador",0);

	// Al planificar busco solo instancias disponibles por lo tanto puede devolver nulo si todavia no se recargo la tabla de instancias
	if (instanciaElegida != NULL) {

		g_tiempoPorEjecucion = g_tiempoPorEjecucion + 1;

		t_conexion* conexionDeInstancia = buscarConexion(g_diccionarioConexiones,
				instanciaElegida->nombre,0);

		enviarStore(conexionDeInstancia->socket, clave);
		//TODO: contemplar posible error en la liberacion de la clave en la instancia , clave inaccesible instancia
		enviarStore(conexionDelPlanificador->socket, clave);

		logTraceSeguro(g_logger, g_mutexLog,
				"ENVIAR STORE planificador %i, clave: %s\n",
				conexionDelPlanificador->socket, clave);
	} else {

		enviarRespuesta(conexionDelPlanificador->socket, ERROR_CLAVE_INACCESIBLE);

	}

	free(paquete);
}

void procesarNombreInstancia(t_paquete* paquete, int cliente_fd) {

	char* nombre = recibirNombreInstancia(paquete);
	t_instancia* instanciaNueva = buscarInstancia( g_tablaDeInstancias,true,nombre, 0);

	if(instanciaNueva == NULL ){
	instanciaNueva = crearInstancia(nombre);
	agregarConexion(g_diccionarioConexiones, instanciaNueva->nombre,cliente_fd);
	agregarInstancia(g_tablaDeInstancias, instanciaNueva);
	}else{
		instanciaNueva->disponible = true;
	}

	distribuirKeys(g_tablaDeInstancias);
	enviarInfoInstancia(cliente_fd, g_configuracion.cantidadEntradas,
			g_configuracion.tamanioEntradas,instanciaNueva->claves);
	logTraceSeguro(g_logger, g_mutexLog, "se conecto instancia: %s", nombre);
	free(paquete);
}

//TODO probar esta funcion
void procesarClaveEliminada(t_paquete* paquete, int cliente_fd){

	char* clave = recibirClaveEliminada(paquete);
	char* nombreInstancia = buscarDiccionarioPorValor(g_diccionarioConexiones,&cliente_fd);
	t_instancia* instanciaElegida = buscarInstancia(g_tablaDeInstancias,false, nombreInstancia, 0);
	eliminiarClaveDeInstancia(instanciaElegida->claves,clave);

}

void procesarNombreESI(t_paquete* paquete, int cliente_fd){

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

void planificador_handler(int signum){
	log_error(g_logger,"Cerrando coordinador\n");
	cerrarTodasLasConexiones(g_diccionarioConexiones);
	exit(0);
}
