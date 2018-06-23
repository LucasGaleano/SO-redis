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
	fflush(stdout);
}

tiempo traerTiempoEjecucion() {
	tiempo tiempoEjecucionAux = g_tiempoPorEjecucion;
	return tiempoEjecucionAux;
}

t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias) {

	//TODO ver si funciona igual con entero

	tiempo fechaMasReciente = traerTiempoEjecucion();
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
	if (cantidadInstanciasDisponibles != 0) {
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
}

t_dictionary* crearDiccionarioConexiones() {

	t_list* aux = list_create();
	return aux;

}

void mostrarDiccionario(t_list* diccionario) {

	void mostrar(t_conexion* conexion) {
		printf("nombre: %s  socket: %i \n", conexion->nombre, conexion->socket);
	}
	sem_wait(&g_mutex_tablas);
	list_iterator(diccionario, mostrar);
	sem_post(&g_mutex_tablas);
}

t_conexion* crearConexion(char* nombre, int socket){
	t_conexion* nuevaConexion = malloc(sizeof(t_conexion));
	nuevaConexion->nombre = string_duplicate(nombre);
	nuevaConexion->socket = socket;
	return nuevaConexion;
}

void agregarConexion(t_list * diccionario, char * nombre, int socket) {


	t_conexion* nuevaConexion = crearConexion(nombre,socket)

	sem_wait(&g_mutex_tablas);
	list_add(diccionario,nuevaConexion);
	sem_post(&g_mutex_tablas);
}

int buscarConexion(t_list * diccionario, char * nombre, int socket) {

	bool igualNombre = true;
	bool igualSocket = true;
	bool conexionCumpleCon(t_conexion* conexion){

		if (nombre != NULL)
			igualNombre = string_equals_ignore_case(conexion->nombre, nombre);

		if (socket != 0)
			igualSocket = (conexion->socket == socket );

			return (igualNombre && igualSocket);

	}
	sem_wait(&g_mutex_tablas);
	t_conexion* conexionBuscada = list_find(diccionario,conexionCumpleCon);
	sem_post(&g_mutex_tablas);
	return conexionBuscada;
}

void eliminiarClaveDeInstancia(t_list* claves, char* claveAEliminar) {

	for (int i = 0; i < list_size(claves); i++) {
		char* clave = list_get(claves, i);
		if (strcmp(clave, claveAEliminar) == 0) {
			list_remove(claves, i);
			break;
		}
	}
}

t_instancia* buscarInstancia(t_list* tablaDeInstancias,bool buscaNoDisponibles, char* nombre,
		int letraAEncontrar) {

	bool instanciaCumpleCon(t_instancia* instancia) {

		bool igualNombre = true;
		bool contieneLetraAEncontrar = true;

		if (nombre != NULL)
			igualNombre = string_equals_ignore_case(instancia->nombre, nombre);

		if (letraAEncontrar != 0)
			contieneLetraAEncontrar = (instancia->primerLetra <= letraAEncontrar
					&& instancia->ultimaLetra >= letraAEncontrar);

		return (igualNombre && contieneLetraAEncontrar && (instancia->disponible || buscaNoDisponibles));

	}

	sem_wait(&g_mutex_tablas);
	t_instancia* instanciaAux = list_find(tablaDeInstancias,(void*) instanciaCumpleCon);
	sem_post(&g_mutex_tablas);
	return instanciaAux;

}

void mostrarTablaInstancia(t_list* tablaDeInstancias) {

	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		mostrarInstancia(instanciaAux);
	}
}

sacarConexion(t_list* diccionario, t_conexion* conexion){

	int index;
	for (index = 0;index<list_size(diccionario);index++){
		t_conexion* aux = list_get(diccionario,index);
		if(strcmp(aux->nombre,conexion->nombre) == 0){
			aux = list_remove(diccionario,index);
			cerrarConexion(aux);
			destruirConexion(aux);
		}
	}
}

void cerrarConexion(t_conexion* conexion) {
	printf("cerrando %s\n",conexion->nombre);
	close(conexion->socket);
}

void destruirConexion(t_conexion* conexion){
		free(conexion->nombre);
		free(conexion);
}

void cerrarTodasLasConexiones(t_list * diccionario) {
	sem_wait(&g_mutex_tablas);
	list_iterate(diccionario, cerrarConexion);
	sem_post(&g_mutex_tablas);
	destruirDiccionario(diccionario);
}

void destruirDiccionario(t_list* diccionario){
	sem_wait(&g_mutex_tablas);
	list_clean_and_destroy_elements(diccionario,destruirConexion)
	sem_post(&g_mutex_tablas);
}
