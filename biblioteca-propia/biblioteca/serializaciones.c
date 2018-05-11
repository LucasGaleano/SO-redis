#include "serializaciones.h"

/*-------------------------Serializacion-------------------------*/
void serializarNumero(t_paquete* unPaquete, int numero) {
	int tamNumero = sizeof(int);

	unPaquete->buffer = malloc(sizeof(t_stream));
	unPaquete->buffer->size = tamNumero;
	unPaquete->buffer->data = malloc(tamNumero);

	memcpy(unPaquete->buffer->data, &numero, tamNumero);
}

void serializarMensaje(t_paquete* unPaquete, char * palabra) {
	int tamPalabra = strlen(palabra) + 1;

	unPaquete->buffer = malloc(sizeof(t_stream));
	unPaquete->buffer->data = malloc(tamPalabra);
	unPaquete->buffer->size = tamPalabra;

	memcpy(unPaquete->buffer->data, palabra, tamPalabra);
}

void serializarHandshake(t_paquete * unPaquete, int emisor) {
	serializarNumero(unPaquete, emisor);
}

void serializarArchvivo(t_paquete * unPaquete, char * rutaArchivo) {
	size_t tamArch;

	FILE * archivofd;

	void * archivo = abrirArchivo(rutaArchivo, &tamArch, &archivofd);

	unPaquete->buffer = malloc(sizeof(t_stream));
	unPaquete->buffer->data = malloc(tamArch);
	unPaquete->buffer->size = tamArch;

	memcpy(unPaquete->buffer->data, archivo, tamArch);

	munmap(archivo, tamArch);

	fclose(archivofd);
}

void serializarClaveValor(t_paquete* unPaquete, t_claveValor* sentencia)
{
	int tamClave = string_length(sentencia->clave) + 1;
	int tamValor = string_length(sentencia->valor) + 1;

	int tamTotal = tamValor+tamClave;

	int desplazamiento = 0;

	unPaquete->buffer = malloc(sizeof(t_stream));
	unPaquete->buffer->size = tamTotal;

	unPaquete->buffer->data = malloc(tamTotal);

	memcpy(unPaquete->buffer->data + desplazamiento, sentencia->clave, tamClave);
	desplazamiento += tamClave;

	memcpy(unPaquete->buffer->data + desplazamiento, sentencia->valor, tamValor);
	desplazamiento += tamValor;
}

void serializarRespuestaStatus(t_paquete* unPaquete, t_respuestaStatus* respuesta)
{
	int tamActual = string_length(respuesta->nomInstanciaActual) + 1;
	int tamPosible = string_length(respuesta->nomIntanciaPosible) + 1;
	int tamValor = string_length(respuesta->valor) + 1;


	int tamTotal = tamValor+tamActual+tamPosible;

	int desplazamiento = 0;

	unPaquete->buffer = malloc(sizeof(t_stream));
	unPaquete->buffer->size = tamTotal;

	unPaquete->buffer->data = malloc(tamTotal);

	memcpy(unPaquete->buffer->data + desplazamiento, respuesta->nomInstanciaActual, tamActual);
	desplazamiento += tamActual;

	memcpy(unPaquete->buffer->data + desplazamiento, respuesta->nomIntanciaPosible, tamPosible);
	desplazamiento += tamPosible;

	memcpy(unPaquete->buffer->data + desplazamiento, respuesta->valor, tamValor);
	desplazamiento += tamValor;
}
/*-------------------------Deserializacion-------------------------*/
int deserializarNumero(t_stream* buffer) {
	return *(int*) (buffer->data);
}

char * deserializarMensaje(t_stream * buffer) {
	char * palabra = strdup(buffer->data);

	return palabra;
}

int deserializarHandshake(t_stream * buffer) {
	return deserializarNumero(buffer);
}

void * deserializarArchivo(t_stream * buffer) {
	void * archivo = malloc(buffer->size);

	memcpy(archivo, buffer->data, buffer->size);

	return archivo;
}

t_claveValor* deserializarClaveValor(t_stream* buffer)
{
	int desplazamiento = 0;
	t_claveValor* ret = malloc(sizeof(t_claveValor));

	ret->clave = strdup(buffer + desplazamiento);
	desplazamiento += string_length(ret->clave);

	ret->valor = strdup(buffer + desplazamiento);
	desplazamiento += string_length(ret->valor);

	return ret;
}

t_respuestaStatus* deserializarRespuestaStatus(t_stream* buffer)
{
	int desplazamiento = 0;
	t_respuestaStatus* ret = malloc(sizeof(t_respuestaStatus));

	ret->nomInstanciaActual = strdup(buffer + desplazamiento);
	desplazamiento += string_length(ret->nomInstanciaActual);

	ret->nomIntanciaPosible = strdup(buffer + desplazamiento);
	desplazamiento += string_length(ret->nomIntanciaPosible);

	ret->valor = strdup(buffer + desplazamiento);
	desplazamiento += string_length(ret->valor);

	return ret;
}
/*-------------------------Funciones auxiliares-------------------------*/
void * abrirArchivo(char * rutaArchivo, size_t * tamArc, FILE ** archivo) {
	//Abro el archivo
	*archivo = fopen(rutaArchivo, "r");

	if (*archivo == NULL) {
		printf("%s: No existe el archivo o el directorio", rutaArchivo);
		return NULL;
	}

	//Copio informacion del archivo
	struct stat statArch;

	stat(rutaArchivo, &statArch);

	//Tamaño del archivo que voy a leer
	*tamArc = statArch.st_size;

	//Leo el total del archivo y lo asigno al buffer
	int fd = fileno(*archivo);
	void * dataArchivo = mmap(0, *tamArc, PROT_READ, MAP_SHARED, fd, 0);

	return dataArchivo;
}
