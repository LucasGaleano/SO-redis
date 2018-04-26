#ifndef SRC_PROCESAMIENTOPAQUETES_H_
#define SRC_PROCESAMIENTOPAQUETES_H_

#include "estructuras.h"
#include "serializaciones.h"

/*-----------------------------------Paquetes-----------------------------------*/
void	 						enviarPaquetes				(int socketfd, t_paquete * unPaquete);
int 							recibirTamPaquete			(int client_socket);
t_paquete * 					recibirPaquete				(int client_socket, int tamPaquete);
t_paquete * 					crearPaquete				(void * buffer);
t_paquete *						crearPaqueteError			(int client_socket);
void 							destruirPaquete				(t_paquete * unPaquete);
void 							mostrarPaquete				(t_paquete * unPaquete);

/*-----------------------------------Enviar paquetes-----------------------------------*/
void 							enviarHandshake				(int server_socket, int emisor);
void 							enviarMensaje				(int server_socket, char * mensaje);
void 							enviarArchivo				(int server_socket, char * rutaArchivo);
void 							enviarSolicitudEjecucion	(int server_socket);
void 							enviarIdentificacion		(int server_socket, char * nombre);

/*-----------------------------------Recibir paquetes-----------------------------------*/
int 							recibirHandshake			(t_paquete * unPaquete);
char * 							recibirMensaje				(t_paquete * unPaquete);
void * 							recibirArchivo				(t_paquete * unPaquete);
char * 							recibirIdentificacion		(t_paquete * unPaquete);

#endif /* SRC_PROCESAMIENTOPAQUETES_H_ */
