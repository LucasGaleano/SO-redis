#include "algoritmos.h"

inline double calcularProximaRafaga(double estimadoAnterior, double realAnterior)
{
	return estimadoAnterior*0.5 + realAnterior*0.5;
}

inline double calcularRR(double tEnEspera, double estimadoAnterior, double realAnterior)
{
	return (1+ tEnEspera/calcularProximaRafaga(estimadoAnterior,realAnterior));
}

inline char* asignarID(int val, char* ret)
{
	char num[2] = {'\0'};
	sprintf(num, "%d", val);
	strcpy(ret, "ESI");
	return strcat(ret, num);
}

char* calcularSiguienteSJF(void)
{
	t_infoListos *actual;
	double menor;

	int i = 0;
	char* auxKey = calloc(5, sizeof(char));
	auxKey = asignarID(i, auxKey);
	char* keyMenor = calloc(5, sizeof(char));
	actual = dictionary_get(g_listos, auxKey);
	menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
	strcpy(keyMenor, auxKey);
	for(i++; i<g_listos->elements_amount; i++)
	{
		auxKey = asignarID(i, auxKey);
		actual = dictionary_get(g_listos, auxKey);
		if(menor > calcularProximaRafaga(actual->estAnterior, actual->realAnterior))
		{
			menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
			strcpy(keyMenor, auxKey);
		}
	}

	return keyMenor;
}

char* calcularSiguienteHRRN(void)
{
	t_infoListos *actual;
		double mayor;

		int i = 0;
		char* auxKey = calloc(5, sizeof(char));
		auxKey = asignarID(i, auxKey);
		char* keyMenor = calloc(5, sizeof(char));
		actual = dictionary_get(g_listos, auxKey);
		mayor = calcularRR(actual->tEnEspera, actual->estAnterior, actual->realAnterior);
		strcpy(keyMenor, auxKey);
		for(i++; i<g_listos->elements_amount; i++)
		{
			auxKey = asignarID(i, auxKey);
			actual = dictionary_get(g_listos, auxKey);
			if(mayor < calcularProximaRafaga(actual->tEnEspera, actual->estAnterior, actual->realAnterior))
			{
				mayor = calcularProximaRafaga(actual->tEnEspera, actual->estAnterior, actual->realAnterior);
				strcpy(keyMenor, auxKey);
			}
		}

		return keyMenor;
}

void envejecer(char* key, t_infoListos* data)
{
	data->tEnEspera += g_tEjecucion;
}

void planificarSinDesalojo(char* algoritmo)
{
	g_termino = 0;
	int cont = 0;
	t_infoListos *aEjecutar;
	char* key;

	if(strcmp(algoritmo, "SJF-SD") == 0)
		key = calcularSiguienteSJF();
	if(strcmp(algoritmo, "SJF-CD") == 0)
			//TODO implementar
	if(strcmp(algoritmo, "HRRN") == 0)
			key = calcularSiguienteHRRN();

	aEjecutar = dictionary_get(g_listos, key);
	while(!g_termino)
	{
		enviarSolicitudEjecucion(aEjecutar->socketESI);
		gestionarSolicitudes(aEjecutar->socketESI, (void*) gestionarRespuestaESI, g_logger);
		cont++;
	}
	if(g_bloqueo)
	{
		t_infoBloqueo* aux = malloc(sizeof(t_infoBloqueo));
		g_bloqueo = 0;
		aEjecutar = dictionary_remove(g_listos, key);
		aux->rafagasYsocket.estAnterior = calcularProximaRafaga(aEjecutar->estAnterior, aEjecutar->realAnterior);
		aux->rafagasYsocket.realAnterior = cont;
		aux->rafagasYsocket.socketESI = aEjecutar->socketESI;
		//aux->codRecurso = codigo que bloquea
		dictionary_put(g_bloq, key, aux);
	}
	if(g_finalizo)
	{
		g_finalizo = 0;
		free(dictionary_remove(g_listos, key));
	}
	g_tEjecucion = cont;
	if(strcmp(algoritmo, "HRRN") == 0)
		dictionary_iterator(g_listos, (void*) envejecer);
	free(key);
}

void gestionarRespuestaESI(t_paquete* unPaquete, int* socket)
{
	//TODO Hablar con Facu sobre como comunicamos resultados de ejecucion ESI
	destruirPaquete(unPaquete);
}
