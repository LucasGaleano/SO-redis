/*
 * algoritmos.h
 *
 *  Created on: 20 abr. 2018
 *      Author: utnso
 */

#ifndef ALGORITMOS_H_
#define ALGORITMOS_H_

//rafaga =rafaga anterior*0,5 + real anterior*0,5
//rr = 1 + espera/rafaga
double calcularProximaRafaga(double, double);
double calcularRR(double, double, double);

#endif /* ALGORITMOS_H_ */
