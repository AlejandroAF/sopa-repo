/*
 * select.h
 *
 *  Created on: 13/05/2013
 *      Author: utnso
 */

#ifndef SELECT_H_
#define SELECT_H_


int sockets_create_Server(int port);
void actualizarDescriptorMaximo(int socketNuevaConexion,int* descr_max);
void agregarNuevaConexionEnVectorClientesConectados(int socketNuevaConexion,int* vectorclientesconectados,int *tamaniovector);
void inicializarVectorEn0(int* vectorclientesconectados,int tamaniovector);
void *threadPlanificador(void *parametro);




#endif /* SELECT_H_ */
