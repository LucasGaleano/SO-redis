double calcularProximaRafaga(double estimadoAnterior, double realAnterior)
{
	return estimadoAnterior*0.5 + realAnterior*0.5;
}

float calcularRR(double tEnEspera, double estimadoAnterior, double realAnterior)
{
	return (1+ tEnEspera/calcularProximaRafaga(estimadoAnterior,realAnterior));
}
