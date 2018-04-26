/*
 * consola.c
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#include "consola.h"

/*------------------------------Consola------------------------------*/
void iniciarConsola(){
	char* linea;
	bool ejecutar = true;  // no se xq no me entiende bool, buscar/preguntar despues

	while(ejecutar){
		linea = readline(">");

		// ver si esto es necesario FACU
		if(linea){
			add_history(linea); // averiguarlo yo despues o preguntar a facu xq le importa saber todos los comandos que se eje
		}
		else {
			free(linea);
			break; // dudoso (continue??)
		}

		ejecutarComando(linea, &ejecutar);

		free(linea);
	}

	clear_history();
}

void ejecutarComando(char* linea, bool* ejecutar){

	// MAN
	if (string_equals_ignore_case(linea, "man")) {
		ejecutarMan();
		return;
	}

	// PAUSAR PLANIFICACIÓN
	if( string_equals_ignore_case(linea, "pausar") ){
		_pausarPlanificacion();
		break;
	}

	// CONTINUAR PLANIFICACIÓN
	if( string_equals_ignore_case(linea, "continuar") ){
		_continuarPlanificacion();
		break;
	}

	// BLOQUEAR ESI
	if( string_equals_ignore_case(linea, "bloquear") ){
		_bloquearESI(linea);
		break;
	}

	// DESBLOQUEAR ESI
	if( string_equals_ignore_case(linea, "desbloquear") ){
		_desbloquearESI(linea);
		break;
	}

	// LISTAR PROCESOS
	if( string_equals_ignore_case(linea, "listar") ){
		_listarProcesos(linea);
		break;
	}

	// KILL PROCESO
	if( string_equals_ignore_case(linea, "kill") ){
		killProceso(linea);
		break;
	}

	// STATUS
	if( string_equals_ignore_case(linea, "status") ){
		status(linea);
		break;
	}

	// DEADLOCK
	if( string_equals_ignore_case(linea, "deadlock") ){
		deadlock();
		break;
	}

	// SALIR DE LA CONSOLA
	if( string_equals_ignore_case(linea, "exit") ){
		salirConsola(ejecutar);
		return;
	}

	// NO RECONOCER COMANDO
	printf("No se ha encontrado el comando %s \n", linea);
}

/*------------------------------Comandos------------------------------*/
void ejecutarMan() {
	printf("NAME \n");
	printf("	consola \n\n");

	printf("SYNOPSIS \n");
	printf("	#include <consola.h> \n\n");

	printf("	void pausar(void) \n");
	printf("	void continuar(void) \n");
	printf("	void bloquear(char* clave, int id \n");
	printf("	void desbloquear(char* clave) \n");
	printf("	void listar(char* recurso \n");
	printf("	void kill(int id) \n");
	printf("	void status(char* clave) \n");
	printf("	void deadlock(void) \n");
	printf("	void exit(void) \n\n");

	printf("DESCRIPTION \n");
	printf("	pausar --> Pausar planificacion \n");
	printf("	continuar --> Continuar planificacion \n");
	printf("	bloquear --> Se bloqueara el proceso ESI hasta ser desbloqueado \n");
	printf("	desbloquear -->  Se desbloqueara el proceso ESI con el ID especificado \n");
	printf("	listar --> Lista los procesos encolados esperando al recurso \n");
	printf("	kill --> Finaliza el proceso \n");
	printf("	status -->  Brinda informacion sobre las instancias del sistema \n");
	printf("	deadlock -->   \n"); // Ver
	printf("	exit --> Cierra la consola \n\n");
}
void salirConsola(bool* ejecutar){
	printf("Se cerro la consola \n");
	*ejecutar = false;
}

void pausarPlanificacion(void){
	pthread_mutex_lock(&mutexConsola);
}

void continuarPlanificacion(void){
	pthread_mutex_unlock(&mutexConsola);
}

void bloquearESI(char* linea){

	// obtener parametros
	char* clave = obtenerParametro(linea, 1);
	char* id_char = obtenerParametro(linea, 2);

	if(clave == NULL)
		return;
	if(id_char == NULL)
		return;

	int id = atoi(id_char);


	// veo que el ESI está listo o ejecutando
	if( estaBloqueado(clave) || estaEjecutando(clave) ){
		puts("Solo se puede bloquear el ESI en estado listo o ejecutando.");
		return;
	}

	t_infoBloqueo aux;
	aux.usuario = true;

	// bloquear proceso
	dictionary_put(g_bloq, clave, aux);

}

void desbloquearESI(char* linea){

	char* clave = obtenerParametro(linea, 1);

	if(clave == NULL)
		return;

	if( !estaBloqueado(clave) ){
		printf("No se puede desbloquear un ESI que no está bloqueado \n");
		break;
	}

	if( !estaBloqueadoPorUsuario(clave) ){
		printf("Solo se puede desbloquear un ESI que fue bloqueado por la consola \n");
		break;
	}

	// desbloquear
	dictionary_remove(g_bloq, clave);
	dictionary_put(g_listos, clave, NULL);
}

void listarProcesos(char* linea){
	// ESI0
	// t_dictionary -> elements_amount cant elementos que hay en total en el diccionario

	char* claveRecurso = obtenerParametro(linea, 1);

	if(claveRecurso == NULL)
		return;

	int i;

	for(i = 0; i < g_bloq->elements_amount; i++) {

		// memdoria dinamica
		// borrale el ultimo el porque siempre voy a agregar nros

		// crear clave
		claveESI = string_append("ESI", itoa(i));

		if( estaBloqueadoPorElRecurso(claveESI, claveRecurso) ){
			printf("%s", claveESI);
		}
	}
}

void killProceso(char* linea){

		int id = obtenerParametro(linea, 1);
		char* id_char = obtenerParametro(linea, 1);

		if(clave == NULL)
			return;

		int id = atoi(id_char);

	// recordar la atomicidad en bloquear



	/*No se si sirve
	 * liberar mapa de memoria
	 * cerrar ficheros y liberar otros recursos
	 * eliminar PCB de cola de procesos listos
	 * liberar PCB
	 * liberar pila del sistema
	 * activa planificador y reliza c. de contexto al proceso elegido*/
}

void status(char* linea){
	// informacion sobre las instancias del sistema

	char* clave = obtenerParametro(linea, 1);

	if(clave == NULL)
		return;
}

void deadlock(){
	// Ver en bloqueos enunciado (p 9)
}


/*------------------------------Auxiliares------------------------------*/

char* obtenerParametro(char* linea, int parametro){
	char** palabras = string_split(linea, " ");
	return palabras[parametro];
}

bool estaBloqueado(char* clave){
	return dictionary_has_key(g_bloq, clave);
}

bool estaEjecutando(char* clave){
	return dictionary_has_key(g_exec, clave);
}

bool estaBloqueadoPorUsuario(char* clave){
	t_infoBloqueo data = dictionary_get(clave);
	return data->usuario;
}

bool estaBloqueadoPorElRecurso(char* claveESI, char* claveRecurso){
	t_infoBloqueo* infoESI = dictionary_get(g_bloq, claveESI);

	return string_equals_ignore_case(infoESI -> codRecurso, claveRecurso);
}
