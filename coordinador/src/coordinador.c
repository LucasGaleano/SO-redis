#include "coordinador.h"
//TODO hacer funcion logSeguro y reemplazar logTraceSeguro
//TODO porque la tabla de instancia tiene socket si esta en el diccionario.
int recibirRespuesta(t_paquete* paquete);
void procesarPaquete(t_paquete* paquete, int* socketCliente);

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
	g_diccionarioConexiones = crearDiccionarioConexiones();

	g_logger = log_create("coordinador.log", "coordinador", true,
			LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(config);

	sem_init(&g_mutexLog, 0, 1); //TODO destrur semaphore
	sem_init(&g_mutex_tablas,0,1);


	iniciarServer(g_configuracion.puertoConexion, (void*) procesarPaquete,
			g_logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete, int* socketCliente) { //TODO destruir paquetes

	pthreadArgs_t* args = malloc(sizeof(pthreadArgs_t));
	args->paquete = paquete;
	args->socket = *socketCliente;
	pthread_t pid;


	switch (paquete->codigoOperacion) {

	case HANDSHAKE:
		log_debug(g_logger,"dentro de procesar el handshake");
		pthread_create(&pid, NULL, procesarHandshake, args);

		break;

	case ENVIAR_NOMBRE_ESI:
		;
		pthread_create(&pid, NULL, procesarNombreESI, args);
		break;

	case ENVIAR_NOMBRE_INSTANCIA:
		;

		pthread_create(&pid, NULL, procesarNombreInstancia, args);
		break;

	case SET:
		;

		pthread_create(&pid, NULL, procesarSET, args);
		break;

	case GET:
		;
		pthread_create(&pid, NULL, procesarGET, args);
		break;

	case STORE:
		;

		//El Coordinador colabora con el Planificador avisando de este recurso
		pthread_create(&pid, NULL, procesarSTORE, args);
		//avisa si hubo error o no por instancia que se desconecto pero tenia la clave
		break;

	case RESPUESTA_SOLICITUD:
		;
		pthread_create(&pid, NULL, procesarRespuesta, args);
		break;

	default:

	break;
	}

}

t_configuraciones armarConfigCoordinador(t_config* archivoConfig) {

	t_configuraciones configuracion;

	configuracion.puertoConexion = config_get_int_value(archivoConfig,
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
		instanciaElegida = traerInstanciaMasEspacioDisponible(tablaDeInstancias);

	if (string_equals_ignore_case(algoritmoDePlanificacion, "EL"))
		instanciaElegida = traerUltimaInstanciaUsada(tablaDeInstancias);

	//TODO algoritmo key explicit "KE"

	if (string_equals_ignore_case(algoritmoDePlanificacion, "KE")) {
		int keyDeClave = (int) string_substring(clave, 0, 1);
		instanciaElegida = buscarInstancia(tablaDeInstancias, NULL, keyDeClave, 0);
	}

	sem_post(&g_mutex_tablas);


	return instanciaElegida;

}


void* procesarRespuesta(void* args) {

	int respuesta = recibirRespuesta(((pthreadArgs_t*)args)->paquete);

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

void* procesarHandshake(void* args) {

	//pthreadArgs_t* argsAux = (pthreadArgs_t*) args;

	log_debug(g_logger,"Entro al procesarHandshake");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;
	int* socketCliente = malloc(sizeof(int));
	*socketCliente = ((pthreadArgs_t*)args)->socket;


	switch (recibirHandshake(paquete)) {
	case PLANIFICADOR:
		log_debug(g_logger,"se conecto el planificador: %i", *socketCliente);
		agregarConexion(g_diccionarioConexiones, "planificador",socketCliente);
		break;

	case ESI:
		//TODO que hacer aca?
		log_info(g_logger,"Se conecto un ESI");
		break;

	case INSTANCIA:
		log_info(g_logger,"Se conecto una instancia");
		break;

	}

	free(paquete);
	free(args);
	return 0;

}

void* procesarSET(void* args) {

	log_debug(g_logger,"Entro al procesarSET");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;

	t_claveValor* sentencia = recibirSet(paquete);

	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, sentencia->clave,
			g_tablaDeInstancias);

	mostrarInstancia(instanciaElegida);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones,
				"planificador");


	usleep(g_configuracion.retardo*1000);
	//sleep(g_configuracion.retardo); esto lo hacia en segundos
	int* socketInstancia = conseguirConexion(g_diccionarioConexiones,instanciaElegida->nombre);

	log_debug("socket a instancia: %i y a planificacion: %i", *socketInstancia, socketDelPlanificador);
	logTraceSeguro(g_logger, g_mutexLog, "a la instancia: %i mando set clave: %s, y se la envia a %s",*socketInstancia,sentencia->clave, instanciaElegida->nombre);

	enviarSet(*socketInstancia, sentencia->clave, sentencia->valor);
	enviarSet(socketDelPlanificador,sentencia->clave,sentencia->valor);


	//TODO si no se puede acceder a la instancia, se le avisa al planificador

	free(paquete);
	free(args);
	return 0;
}


void* procesarGET(void* args) {

	log_debug(g_logger,"Entro al procesarGET");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;

	char* clave = recibirGet(paquete);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones,
			"planificador");

	log_debug(g_logger,"enviar GET al planificador: %i, clave: %s\n",socketDelPlanificador ,clave);

	enviarGet(socketDelPlanificador, clave);

	//free(paquete);
	//free(args);
	return 0;

}

void* procesarSTORE(void* args) {

	log_debug(g_logger,"Entro al procesarSTORE");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;

	char* clave = recibirStore(paquete);

	t_instancia* instanciaElegida = PlanificarInstancia(
				g_configuracion.algoritmoDist, clave,
				g_tablaDeInstancias);

	int socketDelPlanificador = *conseguirConexion(g_diccionarioConexiones, "planificador");

	// Al planificar busco solo instancias disponibles por lo tanto puede devolver nulo si todavia no se recargo la tabla de instancias
	if(instanciaElegida != NULL){

	int* socketInstancia = conseguirConexion(g_diccionarioConexiones,
				instanciaElegida->nombre);

	enviarStore(*socketInstancia, clave);
	//TODO: contemplar posible error en la liberacion de la clave en la instancia , clave inaccesible instancia
	enviarStore(socketDelPlanificador, clave);

	logTraceSeguro(g_logger, g_mutexLog,
			"ENVIAR STORE planificador %i, clave: %s\n",socketDelPlanificador , clave);
	}
	else{

		enviarRespuesta(socketDelPlanificador,ERROR_CLAVE_INACCESIBLE);

	}

	free(paquete);
	free(args);
	return 0;
}

void* procesarNombreInstancia(void* args) {

	log_debug(g_logger,"Entro al procesarNombreInstancia");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;

	int* socketCliente = malloc(sizeof(int));
	*socketCliente = ((pthreadArgs_t*)args)->socket;

	char* nombre = recibirNombreInstancia(paquete);


	t_instancia* instanciaNueva = crearInstancia(nombre, 0);//TODO no guardar socket aca
	agregarConexion(g_diccionarioConexiones, instanciaNueva->nombre,socketCliente);
	agregarInstancia(g_tablaDeInstancias, instanciaNueva);
	distribuirKeys(g_tablaDeInstancias);
	enviarInfoInstancia(socketCliente, g_configuracion.cantidadEntradas,
			g_configuracion.tamanioEntradas);
	logTraceSeguro(g_logger, g_mutexLog, "%s mando el nombre",nombre);

	free(paquete);
	free(args);
	return 0;
}

void* procesarNombreESI(void* args) {

	log_debug(g_logger,"Entro al procesarNombreEsi");
	t_paquete* paquete = ((pthreadArgs_t*)args)->paquete;

	int* socketCliente = malloc(sizeof(int));
	*socketCliente = ((pthreadArgs_t*)args)->socket;

	char* nombreESI = recibirNombreEsi(paquete);

	agregarConexion(g_diccionarioConexiones, nombreESI, socketCliente);
	logTraceSeguro(g_logger, g_mutexLog, "%s mando el nombre",nombreESI);

	free(paquete);
	free(args);

	return 0;
}

void logTraceSeguro(t_log* logger,sem_t mutexLog, char* format, ...) {

	va_list ap;
	va_start(ap, format);
	char* mensaje = string_from_vformat(format, ap);
	sem_wait(&mutexLog);
	log_trace(logger, mensaje);
	sem_post(&mutexLog);
}




