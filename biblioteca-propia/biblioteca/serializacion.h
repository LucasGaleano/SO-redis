#ifndef BIBLIOTECA_SERIALIZACION_H_
#define BIBLIOTECA_SERIALIZACION_H_

#include "estructuras.h"
#include <commons/string.h>
#include <commons/collections/list.h>

/*----------------------------------------Serializacion----------------------------------------*/
void 		serializarNumero		(t_paquete* unPaquete, int numero);
void 		serializarMensaje		(t_paquete* unPaquete, char * mensaje);
void 		serializarHandshake		(t_paquete * unPaquete, int emisor);
void 		serializarArchvivo		(t_paquete * unPaquete, char * rutaArchivo);

/*----------------------------------------Deserializacion----------------------------------------*/
int 		deserializarNumero		(t_stream* buffer);
char * 		deserializarMensaje		(t_stream * buffer);
int 		deserializarHandshake	(t_stream * buffer);
void * 		deserializarArchivo		(t_stream * buffer);

/*----------------------------------------Funciones auxiliares----------------------------------------*/
void * 		abrirArchivo			(char * rutaArchivo, size_t * tamArc, FILE ** archivo);

#endif /* BIBLIOTECA_SERIALIZACION_H_ */
