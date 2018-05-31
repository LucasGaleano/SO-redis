#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "consola.h"
#include "algoritmos.h"

#define RUTA_CONFIGURACION_PLANIF "home/utnso/workspace/configuraciones/planificador.cfg"

#include <biblioteca/sockets.h>
#include <biblioteca/paquetes.h>
#include <biblioteca/estructuras.h>

void procesarPaquete(t_paquete*, int*);
void recibirHandshakePlanif(t_paquete* unPaquete, int* socketCliente);
void planificar(char* algoritmo);
void iniciarServidor(int puerto);
void procesarPaquete(t_paquete* unPaquete, int* socketCliente);
void asignarBloquedas(char** codigos);
void desbloquearESIs(t_infoBloqueo* nodo);
int condicionDeTomada(char* nodo);
void claveEstaTomada(char* key, t_list* value);


#endif /* PLANIFICADOR_H_ */
