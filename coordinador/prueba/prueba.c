#include "prueba.h"
#include <stdarg.h>
#include "commons/string.h"
#include <semaphore.h>



int main(void) {

	sem_init(&g_mutex_tablas,0,1);
/*
	g_ESI = dictionary_create();
	int n = 1;
	agregarConexion(g_ESI,"1",&n);
	int* n1 = conseguirConexion(g_ESI,"1");
	printf("%i\n",*n1);

*


	return 0;
}

void logTraceSeguro(t_log* logger, sem_t mutex, char* format, ...) {

	va_list ap;
	va_start(ap, format);
	char* mensaje = string_from_vformat(format, ap);
	sem_wait(&mutex);
	log_trace(logger, mensaje);
	sem_post(&mutex);

}

void pruebaInstancias(t_list* tablaDeInstancias) {

	agregarInstancia(tablaDeInstancias, crearInstancia("instancia1", 12));
	agregarInstancia(tablaDeInstancias, crearInstancia("instancia2", 13));
	agregarInstancia(tablaDeInstancias, crearInstancia("instancia3", 14));
	mostrarTablaInstancia(tablaDeInstancias);

}
