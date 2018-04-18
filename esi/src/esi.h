#ifndef ESI_H_
#define ESI_H_

#include <biblioteca/sockets.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>

#define RUTA_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/esi.cfg"

/*------------------------Variables globales-------------------------*/
t_log * logESI;

int socketCoordinador;
int socketPlanificador;

/*-------------------------Conexion-------------------------*/
void conectarEsi();
t_config* leerConfiguracion();

#endif /* ESI_H_ */
