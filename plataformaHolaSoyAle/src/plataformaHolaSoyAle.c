/*
 ============================================================================
 Name        : plataformaHolaSoyAle.c
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

#define NUMERO_DE_THREADS 2
//#define DIRECCION "192.168.1.105" //las comillas no son necesarias para definir una constante. En este caso las comillas las pusimos porque queremos que la constante tenga comillas.
#define DIRECCION "127.0.0.1"
#define SIZE_HEADER 20  //tamaño del buffer del header.
#define PUERTOPLANIFICADOR 16000 //usamos el 15000. Hay que tener cuidado de no usar un puerto usado por otro proceso (ojo que el sistema operativo usa muchos).
#define PUERTOORQUESTADOR 15000
#define MAXIMO_CLIENTES 10      // El número máximo de conexiones permitidas  (máximo de clientes conectados simultáneamente a nuestro servidor).
#define REUSE 1
#define MAXIMODENIVELES 256

//tipos
typedef struct nodo{
	void *data;
	struct nodo *next;
} nodo;
struct pndata{
	char ipPlanificador[16];
	int puertoPlanificador;
	char ipNivel[16];
	int puertoNIvel;
} __attribute__((packed));

typedef struct pndata pndata;

//------------------------------------prototipos
//int sockets_create_Server(int);
void *atenderCliente (void *);
int comparar(char*,const char*);
int esperarConexion(int);
void* lanzarHilo(void* paramHilo);
//------------------------------------fin prototipos
//variables globales

pndata niveles[MAXIMODENIVELES];

int main (){
	int socketEscucha,socketNuevaConexion;
	int i;


///////////////////////////////////////////////////////////////////////////////////////
	for(i=0;i<4;i++)
	{
		strcpy(niveles[i].ipNivel,"127.0.0.1");
		strcpy(niveles[i].ipPlanificador,"234.234.234.234");
		niveles[i].puertoNIvel=18000;
		niveles[i].puertoPlanificador=18000;
	}
///////////////////////////////////////////////////////////////////////////////////////
	socketEscucha=sockets_create_Server(PUERTOORQUESTADOR);

	while(1)
	{
		socketNuevaConexion= esperarConexion(socketEscucha);
		lanzarHilo((void*)socketNuevaConexion);
	}

	pthread_exit(0);
}

void *atenderCliente(void *parametro)
{
    time_t ticks=time(NULL);
	char variable[6];
	variable[5]='\0';

	printf("se esta creando el hilo\n");

	recv((int)parametro, variable, 5, 0);

	if (comparar(variable,"NIVEL"))
	{
		recv((int)parametro, variable, 5, 0);
		send((int)parametro,&niveles[atoi(variable+1)], sizeof(pndata), 0);
		close((int)parametro);
		return NULL;
	}
    while(1)
    {

		if (comparar(variable,"REGIS"))
		{
			recv((int)parametro, variable, 5, 0);

			//////////////////////////////////////////////////////////////////////////////
			strcpy(niveles[atoi(variable+1)].ipNivel,"127.0.0.1");
			strcpy(niveles[atoi(variable+1)].ipPlanificador,"127.0.0.1");
			niveles[atoi(variable+1)].puertoNIvel=16000;
			niveles[atoi(variable+1)].puertoPlanificador=16000;
			//////////////////////////////////////////////////////////////////////////////

			send((int)parametro,"REGISTRADO", 11, 0);
		}
		recv((int)parametro, variable, 5, 0);
    }
    printf("%s\n", variable);
    return NULL;
}
int sockets_create_Server(int port)
{
	int socketFD;
	struct sockaddr_in socketInfo;

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		   printf("Error al crear el socket.\n");
		   exit(-1);
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY;
	socketInfo.sin_port = htons(port);
	bzero(&(socketInfo.sin_zero),8);
	printf("Socket creado.\n");

	if (REUSE)
	{
		int on = 1;
		if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
			printf("setsockopt of SO_REUSEADDR error\n");
	}
	// Asociar puerto
	bind(socketFD, (struct sockaddr*) &socketInfo, sizeof(socketInfo));

	if (listen(socketFD, MAXIMO_CLIENTES) == -1) {
		printf("Error al escuchar por el puerto.\n");
	}

	printf("Escuchando conexiones entrantes.\n");

	return socketFD;

}



int esperarConexion(int socketEscucha)
{
	int socketNuevaConexion;
	if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) == -1) //se acepta la conexion
		{
			printf("Error al aceptar conexión.\n");
			return -1;
		}
		//printf("se acepto la conexion \n");
		return socketNuevaConexion;
}

void* lanzarHilo(void* paramHilo)
{
	pthread_t tid;
	pthread_create(&tid,NULL,atenderCliente,paramHilo);
	return 0;
}

int comparar(char *str1,const char *str2)
{//compara si dos cadenas de caracteres son iguales
	return !(strncmp(str1,str2,strlen(str2)));
}


