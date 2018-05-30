
#include "prueba.h"
#include <stdarg.h>
#include "commons/string.h"
#include <semaphore.h>

void logTraceSeguro(t_log* logger,sem_t a,char* format,...);

int main(void) {

  t_list * tablaDeInstancias = crearListaInstancias();
  t_list * listaInstancia = PruebaInstacias(tablaDeInstancias);
  distribuirKeys(listaInstancia);

  t_instancia* aux;
  t_instancia* aux2;
  t_instancia* aux3;
  t_instancia* aux4;
  t_instancia* aux5;
  aux = buscarInstancia(listaInstancia,NULL,97,0);
  aux2 = buscarInstancia(listaInstancia,NULL,105,0);
  aux3 = buscarInstancia(listaInstancia,NULL,122,0);
  aux4 = buscarInstancia(listaInstancia,NULL,124,0);
  aux5 = buscarInstancia(listaInstancia,NULL,109,0);

  printf("%s\n",(aux)->nombre);
  /*
  printf("%s\n",(aux2)->nombre);
  printf("%s\n",(aux3)->nombre);
  //printf("%s\n",(aux4)->nombre);  error no esta en diccionario
  logTraceSeguro(2,"format: %s %i\n",(aux5)->nombre, aux5->socket);
  //mostrarInstancia(list_get(listaInstancia,0));
  //mostrarInstancia(list_get(listaInstancia,1));
  //mostrarInstancia(list_get(listaInstancia,2));
  */

  sem_t a;
  sem_init(&a,0,1);
  t_log* logger = log_create("prueba.log","Prueba",true,LOG_LEVEL_TRACE);
  logTraceSeguro(logger,a,"%s\n",(aux)->nombre);

  return 0;
}

void logTraceSeguro(t_log* logger,sem_t mutex,char* format,...){

	va_list ap;
	va_start(ap,format);
	char* mensaje = string_from_vformat(format,ap);
	sem_wait(&mutex);
	log_trace(logger,mensaje);
	sem_post(&mutex);

}

t_list* PruebaInstacias(t_list* tablaDeInstancias){

  agregarInstancia(tablaDeInstancias,crearInstancia("instancia1",12));
  agregarInstancia(tablaDeInstancias,crearInstancia("instancia2",13));
  agregarInstancia(tablaDeInstancias,crearInstancia("instancia3",14));

  return tablaDeInstancias;
}
