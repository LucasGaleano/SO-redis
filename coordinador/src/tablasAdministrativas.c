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

t_instancia* crearInstancia(char* nombre,int* socket){

   t_instancia* aux = malloc(sizeof (t_instancia));
   aux->nombre = string_duplicate(nombre);
   aux->espacioOcupado = 0;
   aux->socket = socket;
   aux->disponible = true;
   aux->ultimaModificacion = 0;
   aux->primerLetra = 0;
   aux->ultimaLetra = 0;
   aux->trabajoActual = NULL;


   return aux;

}

void destruirInstancia(t_instancia * instancia){
  free(instancia->trabajoActual);
	free(instancia->nombre);
	free(instancia);
}


void mostrarInstancia(t_instancia * instancia){

	printf("nombre: %s\n", instancia->nombre);
}


t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias){

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

t_instancia* traerInstanciaMasEspacioDisponible(t_list* tablaDeInstancias){


    unsigned int espacioMinimo = MAX_ENTRADAS ;
    t_instancia* aux;
    t_instancia* instanciaMenorEspacioOcupado;

    for(int i=0;i<list_size(tablaDeInstancias);i++){

        aux = list_get(tablaDeInstancias,i);
        if(espacioMinimo > aux->espacioOcupado){
          espacioMinimo = aux->espacioOcupado;
          instanciaMenorEspacio = aux;

        }

    }

    return instanciaMenorEspacio;


}
