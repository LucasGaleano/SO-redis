#include "tablasAdministrativas.h"
#include <commons/collections/list.h>

t_list * crearListaInstancias(){

  t_list* aux = list_create();

	return aux;

}

//TODO: ver de no agregar instancia ya existente (mismo nombre)
void agregarInstancia(t_list * lista, instancia* instancia ){

   list_add(lista, instancia);
}

instancia* crearInstancia(char* nombre,int espacio,char* ipPuerto,
							time_t ultimaModificacion,int primerLetra,int ultimaLetra){

   instancia* aux = malloc(sizeof (instancia));
   aux->nombre=string_duplicate(nombre);
   aux->espacio = espacio;
   aux->ipPuerto = string_duplicate(ipPuerto);
   aux->disponible = true;
   aux->ultimaModificacion = ultimaModificacion;
   aux->primerLetra = primerLetra;
   aux->ultimaLetra = ultimaLetra;

   return aux;

}

void destruirInstancia(instancia * instancia){

	free(instancia->nombre);
	free(instancia->ipPuerto);
	free(instancia);
}


void mostrarInstancia(instancia * instancia){

	printf("nombre: %s\n", instancia->nombre);
}




