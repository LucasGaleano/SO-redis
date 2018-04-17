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
};

#endif /* BIBLIOTECA_ESTRUCTURAS_H_ */