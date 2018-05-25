#include "coordinador.h"

int recibirRespuesta(t_paquete* paquete);
void procesarPaquete(t_paquete* paquete,int* socketCliente);
void* imprimir(void* paquete);

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
  g_diccionarioConexiones = crearDiccionarioESI();



	g_logger = log_create("coordinador.log","coordinador",true,LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(config);

	iniciarServer(5555, (void*)procesarPaquete, g_logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete,int* socketCliente){

	pthread_t pid;
	 switch(paquete->codigoOperacion){

		 case HANDSHAKE:

		 		procesarHandshake(paquete, *socketCliente);

			break;

			case ENVIAR_NOMBRE_ESI:

				;

				char* nombreESI = recibirNombreEsi(paquete);
				agregarConexion( g_diccionarioConexiones , nombreESI , *socketCliente);


		 	break;

			case ENVIAR_NOMBRE_INSTANCIA:
				;
				char* nombre = recibirNombreEsi(paquete);
				t_instancia*  instanciaNueva = crearInstancia(nombre,*socketCliente);
				agregarInstancia(g_tablaDeInstancias,instanciaNueva);
				distribuirKeys(g_tablaDeInstancias);

				enviarInfoInstancia(socketCliente,g_configuracion.cantidadEntradas,g_configuracion.tamanioEntradas);


			break;

		 case SET:
		 	;
		  //TODO crear hilo para procesar la conexion
			t_claveValor* sentencia = recibirSET(paquete);
			t_instancia* instanciaElegida = PlanificarInstancia( g_configuracion.algoritmoDist, sentencia->clave, g_tablaDeInstancias);

			enviarSet(instanciaElegida->socket,sentencia->clave,sentencia->valor);

			sleep(g_configuracion.retardo);

			//TODO si no se puede acceder a la instancia, se le avisa al planificador

			break;

		//4- La instancia retorna al Coordinador

		case RESPUESTA_SOLICITUD:
			;
			//El Coordinador logea la respuesta y envía al ESI
			int respuesta = recibirRespuesta(paquete);

			t_instancia* instanciaRespuesta = buscarInstancia(g_tablaDeInstancias,NULL,0,*socketCliente);

			logearRespuesta(respuesta,instanciaRespuesta);

			 break;


		case GET:
			;
			//Envio clave a bloquear al planificador
			enviarGet();
			break;

			case STORE:

				//El Coordinador colabora con el Planificador avisando de este recurso
				//avisa si hubo error o no
				break;

		default:

			break;
	}

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


t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion,char* clave, t_list* tablaDeInstancias){

		if(string_equals_ignore_case(algoritmoDePlanificacion,"LSU"))
			return traerInstanciaMasEspacioDisponible(tablaDeInstancias);

		if(string_equals_ignore_case(algoritmoDePlanificacion,"EL"))
			return traerUltimaInstanciaUsada(tablaDeInstancias);

		//TODO algoritmo key explicit "KE"
		if(string_equals_ignore_case(algoritmoDePlanificacion,"EL"))
			return buscarInstancia(tablaDeInstancias,NULL,(int)string_substring(clave,0,1),NULL);

		return NULL;

}


void logearRespuesta(int respuesta, t_instancia* instancia){

	switch(respuesta){

			case OK:

				log_trace(g_logger,"OK nombre: %s  trabajo: %s\n",instanciaRespuesta->nombre, instanciaRespuesta->trabajoActual  );
				break;

			case ABORTO:

				log_trace(g_logger,"ABORTO nombre: %s  trabajo: %s\n",instanciaRespuesta->nombre, instanciaRespuesta->trabajoActual );
				break;
	}

}


void procesarHandshake(t_paquete* paquete,int socketCliente){

	switch(recibirHandshake(paquete)){
		case PLANIFICADOR:
			agregarConexion(g_diccionarioConexiones,"planificador",socketCliente);
		break;

		case ESI:
			//TODO que hacer aca?
		break;

		case INSTANCIA:

		break;

	}

}
