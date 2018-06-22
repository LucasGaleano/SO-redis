#include "tablasAdministrativas.h"

#define MAX_ENTRADAS 1000

t_list* crearListaInstancias(void) {

	t_list* aux = list_create();
	return aux;

}

//TODO: ver de no agregar instancia ya existente (mismo nombre)
void agregarInstancia(t_list * lista, t_instancia* instancia) {
	sem_wait(&g_mutex_tablas);
	list_add(lista, instancia);
	sem_post(&g_mutex_tablas);
}

t_instancia* crearInstancia(char* nombre) {

	t_instancia* aux = malloc(sizeof(t_instancia));
	aux->nombre = string_duplicate(nombre);
	aux->espacioOcupado = 0;
	aux->disponible = true;
	aux->ultimaModificacion = 0;
	aux->primerLetra = 0;
	aux->ultimaLetra = 0;
	aux->trabajoActual = NULL;
	aux->claves = list_create();

	return aux;

}

void destruirInstancia(t_instancia * instancia) {
	free(instancia->trabajoActual);
	free(instancia->nombre);
	list_destroy(instancia->claves);
	free(instancia);
}

void mostrarInstancia(t_instancia * instancia) {

	printf("nombre: %s\n", instancia->nombre);
	printf("disponible: %d\n", instancia->disponible);
	printf("primerLetra: %d\n", instancia->primerLetra);
	printf("ultimaLetra: %d\n", instancia->ultimaLetra);
	printf("ultimaModificacion: %d\n", instancia->ultimaModificacion);
	printf("\n");
}

tiempo traerTiempoEjecucion(){
	tiempo tiempoEjecucionAux = g_tiempoPorEjecucion;
	return tiempoEjecucionAux;
}

t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias) {

	//TODO ver si funciona igual con entero

	tiempo fechaMasReciente = traerTiempoEjecucion() ;
	t_instancia* aux;
	t_instancia* ultimaInstanciaUsada;

	for (int i = 0; i < list_size(tablaDeInstancias); i++) {

		aux = list_get(tablaDeInstancias, i);
		if (fechaMasReciente > aux->ultimaModificacion && aux->disponible) {
			fechaMasReciente = aux->ultimaModificacion;
			ultimaInstanciaUsada = aux;

		}

	}

	return ultimaInstanciaUsada;
}
t_instancia* traerInstanciaMasEspacioDisponible(t_list* tablaDeInstancias) {

	unsigned int espacioMinimo = MAX_ENTRADAS;
	t_instancia* aux;
	t_instancia* instanciaMenorEspacioOcupado;

	for (int i = 0; i < list_size(tablaDeInstancias); i++) {

		aux = list_get(tablaDeInstancias, i);
		if (espacioMinimo > aux->espacioOcupado && aux->disponible) {
			espacioMinimo = aux->espacioOcupado;
			instanciaMenorEspacioOcupado = aux;

		}

	}

	return instanciaMenorEspacioOcupado;

}

void distribuirKeys(t_list* tablaDeInstancias) { //abecedario en ascci - 97(a) - 122(z)
	int cantidadInstanciasDisponibles = 0;
	int letrasAbecedario = 27;
	int primerLetra = 97;
	int ultimaLetraAbecedario = 122;
	int ultimaLetra = 0;

	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		if (instanciaAux->disponible == true) {
			cantidadInstanciasDisponibles++;
		}
	}
	int letrasPorInstancia = (int) letrasAbecedario
			/ cantidadInstanciasDisponibles;
	int resto = letrasAbecedario
			- (letrasPorInstancia * cantidadInstanciasDisponibles);
	letrasPorInstancia =
			resto == 0 ? letrasPorInstancia : letrasPorInstancia + 1;
	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		if (instanciaAux->disponible == true) {
			instanciaAux->primerLetra = primerLetra;
			ultimaLetra =
					(primerLetra + letrasPorInstancia)
							>= ultimaLetraAbecedario ?
							ultimaLetraAbecedario :
							primerLetra + letrasPorInstancia;
			instanciaAux->ultimaLetra = ultimaLetra;
			primerLetra = ultimaLetra + 1;
		}
	}
}

t_dictionary* crearDiccionarioConexiones() {

	t_dictionary* aux = dictionary_create();
	return aux;

}

void mostrarDiccionario(t_dictionary* diccionario){

	void mostrar(char* key, void* value){
		printf("clave: %s  valor: %i \n", key, *(int*)value );
		fflush(stdout);
	}
	dictionary_iterator(diccionario, mostrar);


}

void agregarConexion(t_dictionary * diccionario, char * clave, int* valor) {
	sem_wait(&g_mutex_tablas);
	if (!dictionary_has_key(diccionario, clave)){
		int* socket = malloc(sizeof(int));
		*socket = *valor;
		dictionary_put(diccionario, clave, socket);
	}

	sem_post(&g_mutex_tablas);
}

int* conseguirConexion(t_dictionary * diccionario, char * clave) {
	sem_wait(&g_mutex_tablas);
	int* aux = dictionary_get(diccionario, clave);
	sem_post(&g_mutex_tablas);
	return aux;
}

//TODO probar esta funcion
char* buscarDiccionarioPorValor(t_dictionary* diccionario, int* valor){

			sem_wait(&g_mutex_tablas);
			char* valorBuscado;

			void buscar(char* key, void* value){
				if(*(int*)value == *valor)
					valorBuscado = key;
			}
		  dictionary_iterator(diccionario, buscar);

			sem_post(&g_mutex_tablas);
			return valorBuscado;
}

void eliminiarClaveDeInstancia(t_list* claves, char* claveAEliminar){

	for(int i=0;i<list_size(claves);i++){
		char* clave = list_get(claves,i);
		if(strcmp(clave,claveAEliminar)==0){
			list_remove(claves,i);
			break;
		}
	}
}

t_instancia* buscarInstancia(t_list* tablaDeInstancias, char* nombre,
		int letraAEncontrar) {

	bool instanciaCumpleCon(t_instancia* instancia) {

		bool igualNombre = true;
		bool contieneLetraAEncontrar = true;

		if (nombre != NULL)
			igualNombre = string_equals_ignore_case(instancia->nombre, nombre);

		if (letraAEncontrar != 0)

			contieneLetraAEncontrar = (instancia->primerLetra <= letraAEncontrar
					&& instancia->ultimaLetra >= letraAEncontrar);

		return (igualNombre && contieneLetraAEncontrar && instancia->disponible);

	}

	sem_wait(&g_mutex_tablas);
	t_instancia* instanciaAux = list_find(tablaDeInstancias,
			(void*) instanciaCumpleCon);
	sem_post(&g_mutex_tablas);
	return instanciaAux;

}

void mostrarTablaInstancia(t_list* tablaDeInstancias) {

	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		mostrarInstancia(instanciaAux);
	}
}

void cerrarTodasLasConexiones(t_dictionary * diccionario){

	void cerrarConexion(void* value){
		close(&value);
		free(value);

	}
	dictionary_clean_and_destroy_elements(diccionario,cerrarConexion);
}
