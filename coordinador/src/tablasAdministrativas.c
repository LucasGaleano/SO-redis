#include "tablasAdministrativas.h"
#include <commons/collections/list.h>


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




