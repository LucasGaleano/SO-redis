#include "coordinador.h"

int recibirRespuesta(t_paquete* paquete);
void procesarPaquete(t_paquete* paquete,int* socketCliente);
void* imprimir(void* paquete);

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
	t_log* logger = log_create("coordinador.log","coordinador",true,LOG_LEVEL_TRACE);

	t_config* config = config_create(PATH_CONFIG);
	g_configuracion = armarConfigCoordinador(config);

	iniciarServer(5555, (void*)procesarPaquete, logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete,int* socketCliente){

	pthread_t pid;
	 switch(paquete->codigoOperacion){

		 case HANDSHAKE:
		 // TODO que no se conecte un planificador mas de 2 veces, ESI y INSTANCIA no creo que sirvan
			switch(recibirHandshake(paquete)){
				case PLANIFICADOR:

				break;

				case ESI:

				break;

				case INSTANCIA:

				break;

			}

			break;

			case ENVIAR_NOMBRE_ESI:
				;


		 	break;

			case ENVIAR_NOMBRE_INSTANCIA:
				;
				char* nombre = recibirNombreEsi(paquete);
				t_instancia*  instanciaNueva = crearInstancia(nombre,*socketCliente);
				agregarInstancia(g_tablaDeInstancias,instanciaNueva);
				distribuirKeys(g_tablaDeInstancias);

				enviarInfoInstancia(socketCliente,g_configuracion.cantidadEntradas,g_configuracion.tamanioEntradas);


			break;

		//1- El Coordinador recibe una solicitud proveniente de un proceso ESI.
		 case SET:
		 	;
		  //TODO crear hilo para procesar la conexion
			//2- El Coordinador procesa la solicitud en su algoritmo de distribución
			//con el fin de determinar la Instancia a la que se le asignará la solicitud.
			t_claveValor* sentencia = recibirSET(paquete);
			t_instancia* instanciaElegida = PlanificarInstancia( g_configuracion.algoritmoDist, sentencia->clave, g_tablaDeInstancias);

			int socketInstancia = instanciaElegida->socket;
			//enviarSet(socketInstancia,sentencia);

			//TODO retardo de planificador

			//TODO si no se puede acceder a la instancia, se le avisa al planificador

			//3- Se elige la Instancia asociada y se le envía la solicitud.



			break;

		//4- La instancia retorna al Coordinador

		case RESPUESTA_SOLICITUD:
			;
			//5- El Coordinador logea la respuesta y envía al ESI
			 break;


		case GET:
			;
			//Envio clave a bloquear al planificador
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


t_instancia* PlanificarInstancia(char* algoritmoDePlanificacion,char* Clave, t_list* tablaDeInstancias){

		if(!strcmp(algoritmoDePlanificacion,"LSU"))
			return traerInstanciaMasEspacioDisponible(tablaDeInstancias);

		if(!strcmp(algoritmoDePlanificacion,"EL"))
			return traerUltimaInstanciaUsada(tablaDeInstancias);

		//TODO algoritmo key explicit "KE"

		return NULL;

}
