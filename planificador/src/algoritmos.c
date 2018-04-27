#include "algoritmos.h"

double calcularProximaRafaga(double estimadoAnterior, double realAnterior)
{
	return estimadoAnterior*0.5 + realAnterior*0.5;
}

double calcularRR(double tEnEspera, double estimadoAnterior, double realAnterior)
{
	return (1+ tEnEspera/calcularProximaRafaga(estimadoAnterior,realAnterior));
}

char* asignarClave(int val)
{
	char *ret = calloc(5, sizeof(char));
	char num = itoa(val);
	strcpy(ret, "ESI");
	return strcat(ret, num);
}

void SJFsinDesalojo()
{
	time_t inicio;
	time_t final;

	t_infoListos *actual;
	double menor;

	int i = 0;
	char* auxKey = asignarClave(i);
	char keyMenor[6];
	actual = dictionary_get(g_listos, auxKey);
	menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
	strcpy(keyMenor, auxKey);
	for(i++; i<g_listos->elements_amount; i++)
	{
		free(auxKey);
		auxKey = asignarClave(i);
		actual = dictionary_get(g_listos, auxKey);
		if(menor > calcularProximaRafaga(actual->estAnterior, actual->realAnterior))
		{
			menor = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
			strcpy(keyMenor, auxKey);
		}
	}

	actual = dictionary_get(g_listos, keyMenor);
	int socket = actual->socketESI;
	inicio = clock();

	enviarSolicitudEjecucion(socket);

	free(dictionary_remove(g_listos, keyMenor));

	//esperar que ESI diga que terminó

	final = clock();

	actual = malloc(sizeof(t_infoListos));
	actual->estAnterior = calcularProximaRafaga(actual->estAnterior, actual->realAnterior);
	actual->realAnterior = final - inicio;
	actual->socketESI = socket;

	dictionary_put(g_listos, keyMenor, actual);

}
