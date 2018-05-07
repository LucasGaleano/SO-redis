#include "tablasAdministrativas.h"

#define MAX_ENTRADAS 1000

t_list* crearListaInstancias(void){

   t_list* aux = list_create();
   return aux;

}

//TODO: ver de no agregar instancia ya existente (mismo nombre)
void agregarInstancia(t_list * lista, t_instancia* instancia ){
   list_add(lista, instancia);
}

t_instancia* crearInstancia(char* nombre,int espacio,char* ipPuerto,
							time_t ultimaModificacion,int primerLetra,int ultimaLetra){

   t_instancia* aux = malloc(sizeof (t_instancia));
   aux->nombre = string_duplicate(nombre);
   aux->espacio = espacio;
   aux->ipPuerto = string_duplicate(ipPuerto);
   aux->disponible = true;
   aux->ultimaModificacion = ultimaModificacion;
   aux->primerLetra = primerLetra;
   aux->ultimaLetra = ultimaLetra;

   return aux;

}

void destruirInstancia(t_instancia * instancia){

	free(instancia->nombre);
	free(instancia->ipPuerto);
	free(instancia);
}


void mostrarInstancia(t_instancia * instancia){

	printf("nombre: %s\n", instancia->nombre);
}


t_instancia* TraerUltimaInstanciaUsada(t_list* tablaDeInstancias){

  time_t fechaMasReciente = time(NULL);
  t_instancia* aux;
  t_instancia* ultimaInstanciaUsada;

  for(int i=0;i<list_size(tablaDeInstancias);i++){

      aux = list_get(tablaDeInstancias,i);
      if(fechaMasReciente > aux->ultimaModificacion){
        fechaMasReciente = aux->ultimaModificacion;
        ultimaInstanciaUsada = aux;

      }

  }

  return ultimaInstanciaUsada;
}

t_instancia* traerInstanciaMenorEspacio(t_list* tablaDeInstancias){


    unsigned int espacioMaximo = MAX_ENTRADAS ;
    t_instancia* aux;
    t_instancia* instanciaMenorEspacio;

    for(int i=0;i<list_size(tablaDeInstancias);i++){

        aux = list_get(tablaDeInstancias,i);
        if(espacioMaximo > aux->espacio){
          espacioMaximo = aux->espacio;
          instanciaMenorEspacio = aux;

        }

    }

    return instanciaMenorEspacio;


}
