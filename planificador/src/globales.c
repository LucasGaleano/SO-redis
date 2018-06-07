#include "globales.h"

static void vaciarClaves(t_list* nodo)
{
	list_destroy_and_destroy_elements(nodo, (void*) free);
}

static void vaciarBloqueados(t_infoBloqueo* nodo)
{
	free(nodo->idESI);
	vaciarClaves(nodo->data);
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
	dictionary_destroy_and_destroy_elements(g_listos, (void*) vaciarBloqueados);
	dictionary_destroy_and_destroy_elements(g_listos, (void*) vaciarClaves);

	exit(0);
}
