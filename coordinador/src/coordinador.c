#include "coordinador.h"


void procesarPaquete(t_paquete* paquete,int socketCliente);

int main(void) {





	// t_list* g_tablaDeInstancias = crearListaInstancias();
	t_log* logger = log_create("coordinador.log","coordinador",true,LOG_LEVEL_TRACE);
	iniciarServer(5555, (void*)procesarPaquete, logger);
	return 0;
}

void procesarPaquete(t_paquete* paquete,int socketCliente){

	switch(paquete->codigoOperacion){

		//1- El Coordinador recibe una solicitud proveniente de un proceso ESI.
		 case SOLICITUD_SET:

		 	printf("%s\n",recibirMensaje(paquete));

			//2- El Coordinador procesa la solicitud en su algoritmo de distribución
			//con el fin de determinar la Instancia a la que se le asignará la solicitud.

			// char* PlanificarInstancia(char* algoritmoDePlanificacion,char* Clave, tablaDeInstancias* tabla);

			// retardo de planificador

			//si no se puede acceder a la instancia, se le avisa al planificador

			//3- Se elige la Instancia asociada y se le envía la solicitud.

			//enviar a instancia.

			// break;

		//4- La instancia retorna al Coordinador

		// case RESPUESTA_SET:
			//5- El Coordinador logea la respuesta y envía al ESI
			 break;


		// case SOLICITUD_GET_STORE:

			//actualizar la tabla de bloques, agregando la clave bloqueada si es que no esta bloqueado
			//El Coordinador colabora con el Planificador avisando de este recurso??????
			//QUIEN CONOCE LAS CLAVES QUE EXISTEN EN EL SISTEMA???????????????????????
			// break;


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

	switch (algoritmoDePlanificacion) {

		case PLANIFICADOR_EL:
			return ultimaInstaciaUsada(tablaDeInstancias);
			break;

		case PLANIFICADOR_LSU:
			return instanciaMayorEspacio(tablaDeInstancias);
			break;
			
		default:
			;
	}


}
