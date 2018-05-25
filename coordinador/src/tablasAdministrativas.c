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

t_instancia* crearInstancia(char* nombre,int socket){

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
	printf("disponible: %d\n", instancia->disponible);
	printf("primerLetra: %d\n", instancia->primerLetra);
	printf("ultimaLetra: %d\n", instancia->ultimaLetra);
	printf("ultimaModificacion: %d\n", instancia->ultimaModificacion);
  printf("\n");
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
          instanciaMenorEspacioOcupado = aux;

        }

    }

    return instanciaMenorEspacioOcupado;


}


void  distribuirKeys (t_list* tablaDeInstancias)
{  //abecedario en ascci - 97(a) - 122(z)
   int cantidadInstanciasDisponibles = 0;
	 int letrasAbecedario = 27;
	 int primerLetra = 97;
	 int ultimaLetraAbecedario = 122;
	 int ultimaLetra = 0;

	 for (size_t i = 0; i <  list_size(tablaDeInstancias); i++) {
    t_instancia* instanciaAux = list_get(tablaDeInstancias,i);
		if (instanciaAux->disponible == true) {
			   cantidadInstanciasDisponibles++;
		}
	 }
	 int letrasPorInstancia = (int)letrasAbecedario/cantidadInstanciasDisponibles;
	 int resto = letrasAbecedario - (letrasPorInstancia * cantidadInstanciasDisponibles);
	 letrasPorInstancia = resto == 0 ? letrasPorInstancia: letrasPorInstancia + 1;
	 for (size_t i = 0; i < list_size(tablaDeInstancias);i++) {
    t_instancia* instanciaAux = list_get(tablaDeInstancias,i);
		if (instanciaAux->disponible == true) {
	 	   instanciaAux->primerLetra = primerLetra;
	     ultimaLetra = (primerLetra + letrasPorInstancia) >= ultimaLetraAbecedario ? ultimaLetraAbecedario: primerLetra + letrasPorInstancia;
		   instanciaAux->ultimaLetra = ultimaLetra;
		   primerLetra = ultimaLetra + 1;
	 }
 	}
}

t_dictionary* crearDiccionarioESI(){

   t_dictionary* aux = dictionary_create();
   return aux;

}

void agregarESI(t_dictionary * diccionario, char * clave , int valor ){
   dictionary_put(diccionario, clave, valor);
}

t_instancia* buscarInstancia(t_list* tablaDeInstancias,char* nombre,int primerLetra,int socket){

  bool instanciaCumpleCon(t_instancia* instancia){

    bool igualNombre = true;
    bool igualPrimerLetra = true;
    bool igualSocket = true;

    if(nombre != NULL)
      igualNombre = string_equals_ignore_case(instancia->nombre,nombre);

    if(socket != 0)
      igualSocket =  instancia->socket == socket ;

    if(primerLetra != 0)
      igualPrimerLetra = instancia->primerLetra == primerLetra ;

    return(igualNombre && igualPrimerLetra && igualSocket && instancia->disponible);

  }

  return list_find(tablaDeInstancias, (void*)instanciaCumpleCon );

}
