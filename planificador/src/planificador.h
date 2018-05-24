#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "consola.h"
#include "algoritmos.h"

#define RUTA_CONFIGURACION_PLANIF "home/utnso/workspace/configuraciones/planificador.cfg"

void procesarPaquete(t_paquete*, int*);
void recibirHandshakePlanif(t_paquete* unPaquete, int* socketCliente);
void planificar(char* algoritmo);
void iniciarServidor(int puerto);
void procesarPaquete(t_paquete* unPaquete, int* socketCliente);
void asignarBloquedas(char** codigos);

#endif /* PLANIFICADOR_H_ */
