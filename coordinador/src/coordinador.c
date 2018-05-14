#include "coordinador.h"

int recibirRespuesta(t_paquete* paquete);
void procesarPaquete(t_paquete* paquete,int* socketCliente);
void* imprimir(void* paquete);

int main(void) {

	g_tablaDeInstancias = crearListaInstancias();
	t_log* logger = log_create("coordinador.log","coordinador",true,LOG_LEVEL_TRACE);
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

				t_instancia*  instanciaAux = crearInstancia();
				crearInstancia(nombre,socketCliente);
				// TODO distribuir key entre instancias para el algoritmo de key explicit
				// agregarInstancia(g_tablaDeInstancias,instanciaAux);
				//TODO Enviar info instancia (cantidades)



			break;

		//1- El Coordinador recibe una solicitud proveniente de un proceso ESI.
		 case SET:
		 	;
		  //TODO crear hilo para procesar la conexion
			//2- El Coordinador procesa la solicitud en su algoritmo de distribución
			//con el fin de determinar la Instancia a la que se le asignará la solicitud.
			t_claveValor* sentencia = recibirSET(paquete);
			t_instancia* instanciaElegida = PlanificarInstancia( g_configuracion.algoritmoDist, sentencia->clave, g_tablaDeInstancias);

			int socketInstancia = *(instanciaElegida->socket);
			enviarSet(socketInstancia,sentencia);

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


void* imprimir(void* paquete){

	printf("mensaje:  %d\n",recibirHandshake(paquete));
	sleep(10);
	pthread_exit((void*)1);
}

void  DistribuirKeys (g_tablaDeInstancias)
{  //abecedario en ascci - 97(a) - 122(z)
   int cantidadInstanciasDisponibles = 0;
	 int letrasAbecedario = 27;
	 int primerLetra = 97;
	 int ultimaLetraAbecedario = 122;
	 int ultimaLetra = 0;

	 for (size_t i = 0; i <  list_size(g_tablaDeInstancias); i++) {
		if (g_tablaDeInstancias[i]->disponible == true) {
			   cantidadInstanciasDisponibles++;
		}
	 }
	 int letrasPorInstancia = (int)letrasAbecedario/cantidadInstanciasDisponibles;
	 int resto = letrasAbecedario - (letrasPorInstancia * cantidadInstanciasDisponibles);
	 letrasPorInstancias = resto == 0 ? letrasPorInstancias: letrasPorInstancias + 1;
	 for (size_t i = 0; i < list_size(g_tablaDeInstancias); i++) {
		if (g_tablaDeInstancias[i]->disponible == true) {
	 	g_tablaDeInstancias[i]-> primeraLetra = primerLetra;
	  ultimaLetra = (primerLetra + letrasPorInstancia) >= ultimaLetraAbecedario ? ultimaLetraAbecedario: primerLetra + letrasPorInstancia;
		g_tablaDeInstancias[i]-> ultimaLetra = ultimaLetra;
		primerLetra = ultimaLetra + 1;
	 }
 	}
}




}
