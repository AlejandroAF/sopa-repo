/*
 ============================================================================
 Name        : nivel.c
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
#include "tad_items.h"

#define DIRECCIONSERVER "127.0.0.1"
#define SIZE_HEADER_SERVER 1024  //tamaño del buffer del header
#define MAXIMO_CLIENTES 3      // El número de conexiones permitidas
#define DIRECCION "127.0.0.1"
#define PUERTO 17000
#define BUFF_SIZE 1024
#define NUMERO_DE_THREADS 1
#define REUSE 1

typedef struct coordenadas{
	int posX;
	int posY;
} coordenadas;

struct registro{
	int nroNivel;
	char ip[16];
	int puerto;
} __attribute__((packed));

typedef struct registro reg;

/////////////////////////////////////////////////PROTOTIPOS//////////////////////////////////////////////////////
int sockets_create_Server(char *ip, int port);
int esperarConexion(int);
int sockets_create_Client(char *ip, int port);
void* lanzarHiloServer(void*);
void* funcionHiloServer(void*);
void chat(int param);
int registrarseEnPlataforma();
int comparar(char *,const char *);
int kbhit();
void crearYDibujarItems(ITEM_NIVEL*);
coordenadas* buscarRecurso(ITEM_NIVEL* lista,char dato);
void leerArchivoDeConfiguracion();
void parseo(char* str);
int devuelveCantidadRecursos(ITEM_NIVEL* lista,char recurso);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////Variables Globales//////////////////////////
int nroNivel,puertoNivel,puertoOrquestador;
char ipOrquestador[16];
ITEM_NIVEL* listaItems=NULL;
////////////////////////////////////////////////////////////

int main()
{
	int socketEscucha,socketNuevaConexion;
	leerArchivoDeConfiguracion();
	registrarseEnPlataforma(listaItems); //comentado para poder probar interacción con personaje en el mismo puerto

////////////////////////////////////PARA PROBAR POSICIONES Y CANTIDADES DE RECURSO Y/O PERSONAJES/////////////////////////////
	CrearPersonaje(&listaItems, '@', 1, 2);
	CrearPersonaje(&listaItems, '#', 3, 4);

	CrearCaja(&listaItems, 'H', 20, 40, 5);
	CrearCaja(&listaItems, 'M', 15, 8, 3);
	CrearCaja(&listaItems, 'F', 9, 19, 2);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//crearYDibujarItems(listaItems); //Además inicializa y dibuja la pantalla

	//¿Liberar memoria consumida para registro?

	socketEscucha=sockets_create_Server(DIRECCIONSERVER,PUERTO);
	while(1)
	{
		socketNuevaConexion = esperarConexion(socketEscucha);
		lanzarHiloServer((void*)socketNuevaConexion);
	}
	pthread_exit(0);

}

int esperarConexion(int socketEscucha)
{
	int socketNuevaConexion;
	if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) == -1) //se acepta la conexion
		{
			printf("Error al aceptar conexión.\n");
			return -1;
		}
		printf("se acepto la conexion \n");
		return socketNuevaConexion;
}

int sockets_create_Server(char *ip, int port) {

	int socketFD;
	struct sockaddr_in socketInfo;

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		   printf("Error al crear el socket.\n");
		   exit(-1);
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ip);
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

void* lanzarHiloServer(void* paramHilo)
{
	pthread_t tid;
	pthread_create(&tid,NULL,funcionHiloServer,paramHilo);
	return 0;
}

void *funcionHiloServer(void *parametro){
	char variable[50];
	char bufferTime[50];
	char personaje[1];
	variable[5]='\0';
	time_t ticks = time(NULL);
	coordenadas* movim = malloc(sizeof(coordenadas));
	printf("se recibe la conexion de un personaje\n");
    while(1)
    {
    	recv((int)parametro, variable, 5, 0);
		if (comparar(variable,"POSRE")) //posicion recurso
		{
				recv((int)parametro, variable, 1, 0);
				send((int)parametro,buscarRecurso(listaItems,variable[0]), sizeof(coordenadas), 0);
		}
		else if(comparar(variable,"REGIS"))
		{
				recv((int)parametro,personaje,1,0);
		}
		else if (comparar(variable,"HORA"))
		{
                snprintf(bufferTime, sizeof(bufferTime), "%.24s\r\n", ctime(&ticks));
				send((int) parametro, bufferTime, strlen(bufferTime), 0);
		}
		else if (comparar(variable,"ENTRE")) //entregar recurso
		{

				recv((int)parametro, variable, 1, 0);

				if(devuelveCantidadRecursos(listaItems,variable[0])>0)
				{
					send((int)parametro,buscarRecurso (listaItems , variable[0]) , sizeof(coordenadas), 0);
					restarRecurso(listaItems , variable[0]);
				}

		}
		else if (comparar(variable,"MOVIM")) //solicitud de movimiento
		{
					recv((int)parametro,movim,sizeof(coordenadas), 0);
					MoverPersonaje(listaItems,personaje[0],movim->posX,movim->posY);
		}
		else if (comparar(variable,"FINIV"))
		{
					BorrarItem(&listaItems,personaje[0]);
		}

    }
    return NULL;
}

// Crear socket
int sockets_create_Client(char *ip, int port)
{

	int socketFD;
	struct sockaddr_in socketInfo;

	printf("Conectando...\n");

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		   printf("Error al crear el socket.\n");
		   exit(-1);
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ip);
	socketInfo.sin_port = htons(port);
	bzero(&(socketInfo.sin_zero),8);
	printf("Socket creado.\n");

	while(connect(socketFD, (struct sockaddr*) &socketInfo, sizeof(socketInfo))==-1)
	{
		printf("Falló la conexión, esperará 1 seg. y se intentará nuevamente\n");
		sleep(1);
		printf("Intentando reconectar...\n");
	}

	return socketFD;
}

int comparar(char *str1,const char *str2)
{
	return !(strncmp(str1,str2,strlen(str2)));
}

void chat(int param)
{
	char buffer[50];
	send(param,"chateando\n",10,0);
	while(1)
	{

		if(recv(param,buffer,50,MSG_DONTWAIT)>0)
		{
			if(strncmp(buffer,"salir",5)==0)
					{
						send(param,"saliendo",10,0);
						break;
					}
					//strlen(buffer);
					printf("%s\n",strtok(buffer,"\r"));
		}

		if(kbhit()>0)
		{
			fgets(buffer,50,stdin);
			send(param,buffer,strlen(buffer),0);
		}

	}
}

int kbhit()
	{
	    struct timeval tv;
	    fd_set fds;
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    FD_ZERO(&fds);
	    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	    return FD_ISSET(STDIN_FILENO, &fds);
	}

int registrarseEnPlataforma()
{
	int socketCliente;
	char buffer[11];
	char bufferRegistro[sizeof(reg)+5];
	reg ale;
	socketCliente = sockets_create_Client(ipOrquestador,15000);
	strcpy(ale.ip,"127.0.0.1");
	ale.nroNivel = nroNivel;
	ale.puerto = puertoNivel;
	memcpy( bufferRegistro,"REGIS",5 );
	memcpy( bufferRegistro+5,&ale,sizeof( reg ) );

	while(1)
	{
		send(socketCliente,bufferRegistro,(sizeof(reg)+5),0);
		recv(socketCliente,buffer,11,0);

		if(comparar(buffer,"REGISTRADO"))
		{
			printf("El nivel se ha registrado exitosamente\n");
			return 1;

		}
		sleep(1);
	}
	return 0;
}

void crearYDibujarItems(ITEM_NIVEL* listaItems)
{
	int rows, cols;
	int q, p;

	int x = 1;
	int y = 1;

	nivel_gui_inicializar();

    nivel_gui_get_area_nivel(&rows, &cols);

	q = rows;
	p = cols;

	CrearPersonaje(&listaItems, '@', q, p);
	CrearPersonaje(&listaItems, '#', x, y);

	CrearCaja(&listaItems, 'H', 20, 40, 5);
	CrearCaja(&listaItems, 'M', 15, 8, 3);
	CrearCaja(&listaItems, 'F', 9, 19, 2);

	nivel_gui_dibujar(listaItems);
}

coordenadas* buscarRecurso(ITEM_NIVEL* lista,char recurso)//busca el recurso pedido por el personaje y devuelve una posicion(struct)
{
 ITEM_NIVEL* aux;
 coordenadas* posRecurso = NULL;
 posRecurso = malloc(sizeof(coordenadas));
 aux=lista;
 while(aux != NULL)
 {
  if((aux->id) == recurso)
  {
	 posRecurso->posX = aux->posx;
	 posRecurso->posY = aux->posy;
     aux = aux->next;
  }
  else
  {
   aux = aux->next;
  }
 }
printf("%d %d\n",posRecurso->posX,posRecurso->posY);
return posRecurso;
}

void parseo(char* str)
{
  char* pch=NULL;
  pch=malloc(sizeof(char)*50);
  pch = strtok(str,"=");
  /*//printf("%s\n",pch);
 // if(comparar(pch,"planDeNiveles"))//
  {
		int i;
		pch = strtok(NULL,"[],");
			for(i=0;pch != NULL;i++)
			{
				vector[i]=pch;
				pch = strtok(NULL,"[],");
			}
  }else*/if(comparar(pch,"nroNivel"))
		  {
	  		pch = strtok(NULL,"\0");
	  		nroNivel = atoi(pch);
		  }
   else if(comparar(pch,"puertoNivel"))
		  {
	   	   	 pch = strtok(NULL,"\0");
	   	     puertoNivel = atoi(pch);
		  }
   else if (comparar(pch,"ipOrquestador"))
   {
	   pch = strtok(NULL,"\0");
	   strcpy(ipOrquestador,pch);
   }
   else if (comparar(pch,"puertoOrquestador"))
   {
		 pch = strtok(NULL,"\0");
		 puertoOrquestador = atoi(pch);
   }
}

void leerArchivoDeConfiguracion()
{
    FILE* arch=NULL;
	char buffer[50];
    if (( arch = fopen("/home/utnso/workspacePruebasTP/nivel/archnivel","r" )) ==NULL)
    {
	   printf("error al abrir del archivo\n");
    }

	fgets(buffer,50,arch); //kjshdfkjshdkjf
	while(!feof(arch))
	{
		parseo(buffer);

		if(fgets(buffer,50,arch) == NULL)
		{
			printf("error al leer del archivo\n");
		}

	}
	printf("nro nivel: %d\n",nroNivel);
	printf("puerto nivel %d\n",puertoNivel);
	printf("puerto orquestador %d\n",puertoOrquestador);
	printf("ip orquestador %s\n",ipOrquestador);
}

int devuelveCantidadRecursos(ITEM_NIVEL* lista,char recurso)
{
 ITEM_NIVEL* aux;
 int cantidad;
 aux=lista;
 while(aux != NULL)
 {
  if((aux->id) == recurso)
  {
	 cantidad = aux->quantity;
     aux = aux->next;
  }
  else
  {
   aux = aux->next;
  }
 }
return cantidad;
}


