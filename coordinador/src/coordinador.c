#include "coordinador.h"


void procesarPaquete(t_paquete* paquete,int socketCliente);
void* imprimir(void* paquete);

int main(void) {


	g_tablaDeInstancias = crearListaInstancias();
	agregarInstancia(g_tablaDeInstancias,(crearInstancia("instancia1",321,"123",321,22,33)));


	// t_list* g_tablaDeInstancias = crearListaInstancias();
	t_log* logger = log_create("coordinador.log","coordinador",true,LOG_LEVEL_TRACE);
	iniciarServer(5555, (void*)procesarPaquete, logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete,int socketCliente){

	 switch(paquete->codigoOperacion){

		 case ENVIAR_MENSAJE:
		 ;
		  pthread_t pid;
		 	pthread_create(&pid,NULL,imprimir,(t_paquete*) paquete);

		 	break;


		//1- El Coordinador recibe una solicitud proveniente de un proceso ESI.
		 case SET:
		  //TODO crear hilo para procesar la conexion
			//2- El Coordinador procesa la solicitud en su algoritmo de distribución
			//con el fin de determinar la Instancia a la que se le asignará la solicitud.
			t_claveValor* sentencia = recibirClaveValor(paquete);
			t_instancia* InstanciaElegida = PlanificarInstancia( configuracion.algoritmoDist,
																				sentencia->clave, g_tablaDeInstancias)
			//TODO retardo de planificador

			//si no se puede acceder a la instancia, se le avisa al planificador

			//3- Se elige la Instancia asociada y se le envía la solicitud.
			char* ip = string_duplicate(InstanciaElegida->ip) //TODO fijarse si hay que hacer free a la ip
			int puerto = InstanciaElegida->puerto;
			int socketInstancia = conectarCliente(ip,puerto,COORDINADOR);
			enviarSentencia(SET,sentencia,InstanciaElegida->ipPuerto, socketInstancia);

			break;

		//4- La instancia retorna al Coordinador

		case RESPUESTA_SOLICITUD:
			//5- El Coordinador logea la respuesta y envía al ESI
			 break;


		case GET:

			//actualizar la tabla de bloques, agregando la clave bloqueada si es que no esta bloqueado
			//El Coordinador colabora con el Planificador avisando de este recurso??????
			break;

			case STORE:

				//actualizar la tabla de bloques, agregando la clave bloqueada si es que no esta bloqueado
				//El Coordinador colabora con el Planificador avisando de este recurso??????
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

		return NULL;



}

void* imprimir(void* paquete){

	printf("%s\n",recibirMensaje(paquete));
	sleep(10);
	pthread_exit((void*)1);
}
