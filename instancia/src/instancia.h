#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <biblioteca/sockets.h>
#include <commons/config.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------Constantes-------------------------*/
#define RUTA_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/instancia.cfg"

/*------------------------Variables globales-------------------------*/
t_log * logInstancia;

int socketCoordinador;

char * algoritmoReemplazo;
char * puntoMontaje;
char * nombreInstancia;
int intervaloDump;

bool recibirSolicitudes;

int cantEntradas;
int tamanioEntrada;

void * almacenamiento;

/*-------------------------Conexion-------------------------*/
void 		conectarInstancia	(void);
t_config* 	leerConfiguracion	(void);

/*-------------------------Procesamiento paquetes-------------------------*/
void 		procesarPaquete				(t_paquete * unPaquete, int * client_socket);
void 		procesarEnviarInfoInstancia	(t_paquete * unPaquete);


#endif /* INSTANCIA_H_ */
