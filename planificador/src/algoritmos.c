double calcularProximaRafaga(double estimadoAnterior, double realAnterior)
{
	return estimadoAnterior*0.5 + realAnterior*0.5;
}

double calcularRR(double tEnEspera, double estimadoAnterior, double realAnterior)
{
	return (1+ tEnEspera/calcularProximaRafaga(estimadoAnterior,realAnterior));
}

void SJFsinDesalojo(double estimadoAnt, double realAnt)
{
	time_t inicio = clock();
	time_t final;


	final = clock();

}
