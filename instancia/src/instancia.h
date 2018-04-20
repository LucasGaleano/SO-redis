#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

// Constante

#define PATH_CONFIG "/home/utnso/workspace/tp-2018-1c--0/configuraciones/instancia.cfg"

// Estructuras

typedef struct instanciaConfig {
t_config * archivoConfig;
int coordinadorPuertoConfig;
char* coordinadorIpConfig;
char* algoritmoReemplazo;
char* puntoMontaje;
char* nombreInstancia;
int intervaloDump;
}instanciaConfig;

// Funciones

instanciaConfig cargarConfiguracionInstancia(char* pathConfig);


#endif /* INSTANCIA_H_ */
