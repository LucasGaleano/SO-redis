#ifndef BIBLIOTECA_ESTRUCTURAS_H_
#define BIBLIOTECA_ESTRUCTURAS_H_

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*------------------------------Estructuras------------------------------*/
#define TAM_BLOQUE 1048576

typedef struct {
	size_t size;
	void * data;
} t_stream;

typedef struct {
	int codigoOperacion;
	t_stream * buffer;
}t_paquete;

enum emisor {
	COORDINADOR = 900,
	ESI = 901,
	INSTANCIA = 902,
	PLANIFICADOR = 903,
};

enum cod_op{
	HANDSHAKE=0,
	ENVIAR_MENSAJE,
	ENVIAR_ARCHIVO,
	ENVIAR_ERROR,

	SOLICITUD_EJECUCION,

	ENVIAR_IDENTIFICACION,

	GET,
	SET,
	STORE,

	RESPUESTA_SOLICITUD,

	SOLICITAR_STATUS,
	RESPUESTA_STATUS,
};

enum cod_respuesta
{
	OK = 0,
	ABORTO,
};

//------------------------------Estructuras de comunicacion ESI Coordinador Instancia------------------------------//

typedef struct
{
	char* clave, valor;
}t_claveValor;


//------------------------------Estructuras de comunicacion Consola Instancia------------------------------//

typedef struct
{
	char* valor, nomInstanciaActual, nomIntanciaPosible;
}t_respuestaStatus;

#endif /* BIBLIOTECA_ESTRUCTURAS_H_ */
