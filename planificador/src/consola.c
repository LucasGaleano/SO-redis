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
	// PAUSAR PLANIFICACIÓN
	if( string_equals_ignore_case(linea, "pausar") ){ // no estoy segura del nombre pausar
		pausarPlanificacion();
		break;
	}

	// CONTINUAR PLANIFICACIÓN
	if( string_equals_ignore_case(linea, "continuar") ){ // no estoy segura del nombre continuar
		continuarPlanificacion();
		break;
	}

	// BLOQUEAR ESI
	if( string_equals_ignore_case(linea, "bloquear") ){ // no estoy segura del nombre continuar
		bloquearESI(linea);
		break;
	}

	// DESBLOQUEAR ESI
	if( string_equals_ignore_case(linea, "desbloquear") ){
		desbloquearESI(linea);
		break;
	}

	// LISTAR

	// KILL PROCESO

	// STATUS

	// DEADLOCK

	// FORMATER FILESYSTEM ?? ver después o preguntar

	// SALIR DE LA CONSOLA
	if( string_equals_ignore_case(linea, "exit") ){
		salirConsola(ejecutar);
		return;
	}
	// NO RECONOCER COMANDO
	printf("No se ha encontrado el comando %s \n", linea);
}

/*------------------------------Comandos------------------------------*/
void salirConsola(bool* ejecutar){
	*ejecutar = false;
	puts("Se está saliendo de la consola");

}

void pausarPlanificacion(void){
	// syscall bloqueante que espere la entrada de un humano
	// MIRAR bien despues
}

void continuarPlanificacion(void){
	// idem pausar
}

void bloquearESI(char* linea){
	// ver que el ESI está listo o ejecutando
	if( /* está en un estado que no es listo o ejecutando*/ ){
		puts("Solo se puede bloquear el ESI en estado listo o ejecutando.");
		return;
	}

	// obtener parametros
	char* clave = obtenerParametro(linea, 1); // ver/pregustar xq facu pone todo path
	/*tipo_id*/ id = obtenerParametro(linea, 2); // ver tipo de dato ID = char* ??

	// no estoy segura:

	if(clave == NULL)
		return;
	if(id == NULL)
		return;

	// poner en bloqueados
	// VEEEEEEER viendo el archivo esi

}

void desbloquearESI(char* linea){
	if( /*el ESI no esta bloqueado*/ ){
		puts("No se puede desbloquear un ESI que no está bloqueado");
		break;
	}

	if( /*un ESI está esperando un recurso*/ ){
		puts("No se puede desbloquear un ESI que está esperando un recurso");
	}

	/*tipo_id*/ id = obtenerParametro(linea, 1);

	// desbloquear

}


/*------------------------------Auxiliares------------------------------*/
char* obtenerParametro(char* linea, int parametro){
	// ver
}
