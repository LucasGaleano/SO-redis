#include "coordinador.h"

int recibirRespuesta(t_paquete* paquete);
void procesarPaquete(t_paquete* paquete, int* socketCliente);
void* imprimir(void* paquete);

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
	g_diccionarioConexiones = crearDiccionarioConexiones();

	g_logger = log_create("coordinador.log", "coordinador", true,
			LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(config);

	sem_init(&g_mutexLog, 0, 1); //TODO destrur semaphore

	iniciarServer(g_configuracion.puertoConexion, (void*) procesarPaquete,
			g_logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete, int* socketCliente) { //TODO destruir paquetes

	char* clave;
	pthread_t pid;
	switch (paquete->codigoOperacion) {

	case HANDSHAKE:

		procesarHandshake(paquete, *socketCliente);

		break;

	case ENVIAR_NOMBRE_ESI:
		;

		char* nombreESI = recibirNombreEsi(paquete);
		procesarNombreESI(nombreESI, *socketCliente);
		break;

	case ENVIAR_NOMBRE_INSTANCIA:
		;
		char* nombre = recibirNombreEsi(paquete);
		procesarNombreInstancia(nombre, *socketCliente);
		break;

	case SET:
		;

		//TODO crear hilo para procesar la conexion
		t_claveValor* sentencia = recibirSet(paquete);
		procesarSET(sentencia, *socketCliente);
		break;

	case RESPUESTA_SOLICITUD:
		;
		//El Coordinador logea la respuesta y envía al ESI
		int respuesta = recibirRespuesta(paquete);
		procesarRespuestaSET(respuesta, *socketCliente);
		break;

	case GET:
		;
		clave = recibirGet(paquete);
		procesarGET(clave, *socketCliente);
		break;

	case STORE:

		clave = recibirStore(paquete);
		//El Coordinador colabora con el Planificador avisando de este recurso
		procesarSTORE(clave, *socketCliente);
		//avisa si hubo error o no por instancia que se desconecto pero tenia la clave
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

	if (string_equals_ignore_case(algoritmoDePlanificacion, "LSU"))
		return traerInstanciaMasEspacioDisponible(tablaDeInstancias);

	if (string_equals_ignore_case(algoritmoDePlanificacion, "EL"))
		return traerUltimaInstanciaUsada(tablaDeInstancias);

	//TODO algoritmo key explicit "KE"

	if (string_equals_ignore_case(algoritmoDePlanificacion, "EL"))
		return buscarInstancia(tablaDeInstancias, NULL,
				(int) string_substring(clave, 0, 1), 0);

	return NULL;

}

void logearRespuesta(int respuesta, t_instancia* instanciaRespuesta) {

	switch (respuesta) {

	case OK:

		logTraceSeguro(g_logger, g_mutexLog, "OK nombre: %s  trabajo: %s\n",
				instanciaRespuesta->nombre, instanciaRespuesta->trabajoActual);
		break;

	case ABORTO:

		logTraceSeguro(g_logger, g_mutexLog, "ERROR nombre: %s  trabajo: %s\n",
				instanciaRespuesta->nombre, instanciaRespuesta->trabajoActual);
		break;
	}
}

void procesarHandshake(t_paquete* paquete, int socketCliente) {

	switch (recibirHandshake(paquete)) {
	case PLANIFICADOR:
		agregarConexion(g_diccionarioConexiones, "planificador", socketCliente);
		break;

	case ESI:
		//TODO que hacer aca?
		break;

	case INSTANCIA:

		break;

	}

}

void procesarSET(t_claveValor* sentencia, int socketCliente) {

	t_instancia* instanciaElegida = PlanificarInstancia(
			g_configuracion.algoritmoDist, sentencia->clave,
			g_tablaDeInstancias);
	sleep(g_configuracion.retardo);
	enviarSet(instanciaElegida->socket, sentencia->clave, sentencia->valor);
logTraceSeguro(g_logger, g_mutexLog, "ENVIAR SET planificador clave: %s\n",sentencia->clave);
//TODO si no se puede acceder a la instancia, se le avisa al planificador
}

void procesarRespuestaSET(int respuesta, int socketCliente) {

t_instancia* instanciaRespuesta = buscarInstancia(g_tablaDeInstancias, NULL, 0,
		socketCliente);
logearRespuesta(respuesta, instanciaRespuesta);
}

void procesarGET(char* clave, int socketCliente) {

int socketDelPlanificador = conseguirConexion(g_diccionarioConexiones,
		"planificador");
enviarGet(socketDelPlanificador, clave);
logTraceSeguro(g_logger, g_mutexLog, "ENVIAR GET planificador clave: %s\n",clave);
}

void procesarSTORE(char* clave, int socketCliente) {

int socketDelPlanificador = conseguirConexion(g_diccionarioConexiones,
	"planificador");
enviarStore(socketDelPlanificador, clave);
logTraceSeguro(g_logger, g_mutexLog, "ENVIAR STORE planificador clave: %s\n",clave);
}

void procesarNombreInstancia(char* nombre, int socketCliente) {

t_instancia* instanciaNueva = crearInstancia(nombre, socketCliente);
agregarInstancia(g_tablaDeInstancias, instanciaNueva);
distribuirKeys(g_tablaDeInstancias);
enviarInfoInstancia(socketCliente, g_configuracion.cantidadEntradas,
g_configuracion.tamanioEntradas);
}

void procesarNombreESI(char* nombreESI, int socketCliente) {
agregarConexion(g_diccionarioConexiones, nombreESI, socketCliente);
}

void logTraceSeguro(t_log* logger, sem_t mutexLog, char* format, ...) {

va_list ap;
va_start(ap, format);
char* mensaje = string_from_vformat(format, ap);
sem_wait(&mutexLog);
log_trace(logger, mensaje);
sem_post(&mutexLog);
}
