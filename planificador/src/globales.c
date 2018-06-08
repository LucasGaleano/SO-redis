#include "globales.h"

static void vaciarListaBloqueados(t_infoBloqueo* nodo)
{
	free(nodo->idESI);
	free(nodo->data);
	free(nodo);
}

static void vaciarBloqueados(t_list* nodo)
{
	list_destroy_and_destroy_elements(nodo, (void*)vaciarListaBloqueados);
}

static void vaciarClaves(t_list* nodo)
{
	list_destroy_and_destroy_elements(nodo, (void*)free);
}
extern void liberarTodo(void)
{
	pthread_cancel(&hiloAlgoritmos);
	pthread_cancel(&hiloServidor);
	pthread_join(&hiloAlgoritmos, NULL);
	pthread_join(&hiloServidor, NULL);

	pthread_mutex_destroy(&mutexBloqueo);
	pthread_mutex_destroy(&mutexConsola);
	pthread_mutex_destroy(&mutexListo);
	pthread_mutex_destroy(&mutexLog);
	pthread_mutex_destroy(&modificacion);
	sem_destroy(&ESIentrada);
	sem_destroy(&continua);

	free(g_algoritmo);
	log_destroy(g_logger);
	config_destroy(g_con);

	dictionary_destroy_and_destroy_elements(g_listos, (void*) free);
	dictionary_destroy_and_destroy_elements(g_bloq, (void*) vaciarBloqueados);
	dictionary_destroy_and_destroy_elements(g_clavesTomadas, (void*) vaciarClaves);

	exit(0);
}
