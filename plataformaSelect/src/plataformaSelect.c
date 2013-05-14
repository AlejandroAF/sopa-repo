/*
 ============================================================================
 Name        : plataformaSelect.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "select.h"

#define PUERTOBASE_PLANIFICADOR 16000 //usamos el 15000. Hay que tener cuidado de no usar un puerto usado por otro proceso (ojo que el sistema operativo usa muchos).
#define PUERTOORQUESTADOR 15000
#define MAXIMO_CLIENTES 10      // El número máximo de conexiones permitidas  (máximo de clientes conectados simultáneamente a nuestro servidor).
#define MAXIMODENIVELES 256

//tipos
typedef struct nodo{
	void *data;
	struct nodo *next;
} nodo;

typedef struct hiloplanificador{
	int puerto;
	int conexionNivel;
} hplan;

struct registro{
	int nroNivel;
	char ip[16];
	int puerto;
} __attribute__((packed));

typedef struct registro reg;

struct pndata{
	char ipPlanificador[16];
	int puertoPlanificador;
	char ipNivel[16];
	int puertoNIvel;
} __attribute__((packed));

typedef struct pndata pndata;


//------------------------------------prototipos
int sockets_create_Server(int);
void *hiloPlanificador (void *);
int comparar(char*,const char*);
int esperarConexion(int socketEscucha,struct sockaddr* datosDelCliente);
void registrarNivelYlanzarHiloDePlanificador(int socketConexion);
//------------------------------------fin prototipos
//variables globales

pndata niveles[MAXIMODENIVELES];

int main (){
	int socketEscucha,socketNuevaConexion;
	char buffer[10];

	socketEscucha=sockets_create_Server(PUERTOORQUESTADOR);

	while(1)
	{
		socketNuevaConexion= esperarConexion(socketEscucha,NULL);
		recv(socketNuevaConexion,buffer,5,MSG_PEEK);
		if(comparar(buffer,"NIVEL"))
		{
			recv(socketNuevaConexion, buffer, 10, 0);
			send(socketNuevaConexion,&niveles[atoi(buffer+6)], sizeof(pndata), 0);
			close(socketNuevaConexion);
		} else if(comparar(buffer,"REGIS"))
		{
			registrarNivelYlanzarHiloDePlanificador(socketNuevaConexion);
		}

	}

	pthread_exit(0);
}

void *hiloPlanificador(void *parametro)
{
	char variable[6];
	variable[5]='\0';
	reg estrucRegistro;

	printf("Nuevo hilo \n");

	recv((int)parametro, variable, 5, 0);

    while(1)
    {

		if (comparar(variable,"REGIS"))
		{
			printf("Recibiendo datos de registro \n");
			recv((int)parametro, &estrucRegistro, sizeof(reg), 0);

			//////////////////////////////////////////////////////////////////////////////
			strcpy(niveles[estrucRegistro.nroNivel].ipNivel,estrucRegistro.ip);
			strcpy(niveles[estrucRegistro.nroNivel].ipPlanificador,"127.0.0.1");
			niveles[estrucRegistro.puerto].puertoNIvel=estrucRegistro.puerto;
			niveles[estrucRegistro.nroNivel].puertoPlanificador=16000;
			//////////////////////////////////////////////////////////////////////////////

			send((int)parametro,"REGISTRADO", 11, 0);
			printf("Nivel registrado exitosamente \n");
		}
		recv((int)parametro, variable, 5, 0);
    }
    printf("%s\n", variable);
    return NULL;
}


int esperarConexion(int socketEscucha,struct sockaddr* datosDelCliente)
{
	int socketNuevaConexion;

	socklen_t len= sizeof(struct sockaddr_in);
	if ((socketNuevaConexion = accept(socketEscucha, (struct sockaddr*)datosDelCliente, &len)) == -1) //se acepta la conexion
		{
			printf("Error al aceptar conexión.\n");
			return -1;
		}

	//printf("se acepto la conexion \n");
	return socketNuevaConexion;
}

void registrarNivelYlanzarHiloDePlanificador(int socketConexion)
{
	reg estructuraRegistro;
	char buffer[5];
	recv(socketConexion,buffer,5,0);
	recv(socketConexion,&estructuraRegistro,sizeof(reg),0);

	//registroNivel
	strcpy(niveles[estructuraRegistro.nroNivel].ipNivel,estructuraRegistro.ip);
	niveles[estructuraRegistro.nroNivel].puertoNIvel=estructuraRegistro.puerto;
	strcpy(niveles[estructuraRegistro.nroNivel].ipPlanificador,"127.0.0.1");
	niveles[estructuraRegistro.nroNivel].puertoPlanificador=estructuraRegistro.nroNivel+PUERTOBASE_PLANIFICADOR;
	//fin registro nivel

	//levanto thread planificador y le paso el puerto de escucha
	pthread_t tid;
	hplan *estructura=malloc(sizeof(hplan));
	estructura->puerto=estructuraRegistro.nroNivel+PUERTOBASE_PLANIFICADOR;
	estructura->conexionNivel=socketConexion;
	pthread_create(&tid,NULL,threadPlanificador,(void*)estructura);
}

int comparar(char *str1,const char *str2)
{//compara si dos cadenas de caracteres son iguales
	return !(strncmp(str1,str2,strlen(str2)));
}

void obtenerIP(char ip[16],struct sockaddr_in datosDelCliente)
{
	inet_ntop( AF_INET, &(datosDelCliente.sin_addr.s_addr), ip, INET_ADDRSTRLEN );
}
