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

t_instancia* crearInstancia(char* nombre,int cantidadEntradas, int tamanioEntrada) {

	t_instancia* aux = malloc(sizeof(t_instancia));
	aux->nombre = string_duplicate(nombre);
	aux->espacioMaximo = cantidadEntradas;
	aux->espacioOcupado = 0;
	aux->tamanioEntrada = tamanioEntrada;
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
	printf("claves: \n");
	for(int i=0;i<list_size(instancia->claves);i++)
		printf("  %s\n",(char*)list_get(instancia->claves,i));
	printf("\n");
	fflush(stdout);
}

tiempo traerTiempoEjecucion() {
	tiempo tiempoEjecucionAux = g_tiempoPorEjecucion;
	return tiempoEjecucionAux;
}

t_instancia* traerUltimaInstanciaUsada(t_list* tablaDeInstancias) {

	tiempo fechaMasReciente = traerTiempoEjecucion();
	t_instancia* aux = NULL;
	t_instancia* ultimaInstanciaUsada = NULL;

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
	t_instancia* aux = NULL;
	t_instancia* instanciaMenorEspacioOcupado = NULL;

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

t_list* crearDiccionarioConexiones() {

	t_list* aux = list_create();
	return aux;

}

void mostrarDiccionario(t_list* diccionario) {

	void mostrar(void* conexion) {
		t_conexion* conexionAMostrar = (t_conexion*) conexion;
		printf("nombre: %s  socket: %i \n", conexionAMostrar->nombre, conexionAMostrar->socket);
	}
	sem_wait(&g_mutex_tablas);
	list_iterate(diccionario, mostrar);
	sem_post(&g_mutex_tablas);
}

t_conexion* crearConexion(char* nombre, int socket){
	t_conexion* nuevaConexion = malloc(sizeof(t_conexion));
	nuevaConexion->nombre = string_duplicate(nombre);
	nuevaConexion->socket = socket;
	return nuevaConexion;
}

void agregarConexion(t_list * diccionario, char * nombre, int socket) {


	t_conexion* nuevaConexion = crearConexion(nombre,socket);

	sem_wait(&g_mutex_tablas);
	list_add(diccionario,nuevaConexion);
	sem_post(&g_mutex_tablas);
}

t_conexion* buscarConexion(t_list * diccionario, char * nombre, int socket) {

	bool igualNombre = true;
	bool igualSocket = true;
	bool conexionCumpleCon(void* conexion){
		t_conexion* conexionBuscar = (t_conexion*) conexion;

		if (nombre != NULL)
			igualNombre = string_equals_ignore_case(conexionBuscar->nombre, nombre);

		if (socket != 0)
			igualSocket = (conexionBuscar->socket == socket );

			return (igualNombre && igualSocket);

	}
	sem_wait(&g_mutex_tablas);
	t_conexion* conexionBuscada = list_find(diccionario,conexionCumpleCon);
	sem_post(&g_mutex_tablas);
	return conexionBuscada;
}

void eliminiarClaveDeInstancia(t_instancia* instancia, char* claveAEliminar) {

	t_list* claves = instancia->claves;
	for (int i = 0; i < list_size(claves); i++) {
		char* clave = list_get(claves, i);
		if (strcmp(clave, claveAEliminar) == 0) {
			char* claveEliminada = list_remove(claves, i);
			int entradasOcupadas = cantidadEntradasPorClave(clave,instancia->tamanioEntrada);
			instancia->espacioOcupado -= entradasOcupadas;
			free(claveEliminada);
			break;
		}
	}
}

void agregarClaveDeInstancia(t_instancia* instancia, char* clave){

	char *claveAgregar = string_duplicate(clave);
	list_add(instancia->claves,claveAgregar);
	int entradasOcupadas = cantidadEntradasPorClave(clave,instancia->tamanioEntrada);
	instancia->espacioOcupado += entradasOcupadas;
}

int cantidadEntradasPorClave(char* clave, int tamanioEntrada){
	int lenClave = string_length(clave);
	return (lenClave/tamanioEntrada + (lenClave%tamanioEntrada != 0));
}

t_instancia* buscarInstancia(t_list* tablaDeInstancias,bool buscaNoDisponibles, char* nombre,
		int letraAEncontrar, char* clave) {

		bool igualNombre = true;
		bool contieneLetraAEncontrar = true;
		bool contieneClave = true;

		bool instanciaCumpleCon(t_instancia* instancia) {

		if (nombre != NULL)
			igualNombre = string_equals_ignore_case(instancia->nombre, nombre);

		if (letraAEncontrar != 0)
			contieneLetraAEncontrar = (instancia->primerLetra <= letraAEncontrar
					&& instancia->ultimaLetra >= letraAEncontrar);
		if(clave != NULL){
			contieneClave = instanciaContieneClave(instancia->claves,clave);
		}

		return (igualNombre && contieneLetraAEncontrar && contieneClave &&(instancia->disponible || buscaNoDisponibles));

	}
	sem_wait(&g_mutex_tablas);
	t_instancia* instanciaAux = list_find(tablaDeInstancias,(void*) instanciaCumpleCon);
	sem_post(&g_mutex_tablas);
	return instanciaAux;
}

bool instanciaContieneClave(t_list* claves,char* clave){

	bool contieneClave = false;
	for(int i=0;i<list_size(claves);i++){
		if(strcmp(list_get(claves,i),clave)==0){
			contieneClave = true;
			break;
		}
	}
	return contieneClave;
}

void mostrarTablaInstancia(t_list* tablaDeInstancias) {

	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		mostrarInstancia(instanciaAux);
	}
	for (size_t i = 0; i < list_size(tablaDeInstancias); i++) {
		t_instancia* instanciaAux = list_get(tablaDeInstancias, i);
		mostrarEspacioOcupado(instanciaAux);
	}
	printf("\n");
}

void mostrarEspacioOcupado(t_instancia* instancia){
	char* espacio = string_repeat('|',instancia->espacioOcupado);
	printf("%s %i/%i [", instancia->nombre, instancia->espacioOcupado,instancia->espacioMaximo);
	printf("%-*s]\n",instancia->espacioMaximo,espacio);
	free(espacio);
}

void sacarConexion(t_list* diccionario, t_conexion* conexion){
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

void cerrarConexion(void* conexion) {
	t_conexion* conexionACerrar = (t_conexion*)conexion;
	printf("cerrando %s\n",conexionACerrar->nombre);
	close(conexionACerrar->socket);
}

void destruirConexion(void* conexion){
		t_conexion* conexionACerrar = (t_conexion*)conexion;
		free(conexionACerrar->nombre);
		free(conexionACerrar);
}

void cerrarTodasLasConexiones(t_list * diccionario) {
	sem_wait(&g_mutex_tablas);
	list_iterate(diccionario, cerrarConexion);
	sem_post(&g_mutex_tablas);
	destruirDiccionario(diccionario);
}

void destruirDiccionario(t_list* diccionario){
	sem_wait(&g_mutex_tablas);
	list_clean_and_destroy_elements(diccionario,destruirConexion);
	sem_post(&g_mutex_tablas);
}
