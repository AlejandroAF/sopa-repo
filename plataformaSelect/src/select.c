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

typedef struct hiloplanificador{
	int puerto;
	int conexionNivel;
} hplan;

int sockets_create_Server(int port) {

	int socketFD;
	int backlog=10;
	struct sockaddr_in socketInfo;

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		   printf("Error al crear el socket.\n");
		   exit(-1);
	}

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY;
	socketInfo.sin_port = htons(port);
	bzero(&(socketInfo.sin_zero),8);
	printf("Socket creado.\n");

	int on = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		printf("setsockopt of SO_REUSEADDR error\n");

	// Asociar puerto
	bind(socketFD, (struct sockaddr*) &socketInfo, sizeof(socketInfo));

	if (listen(socketFD, backlog) == -1) {
		printf("Error al escuchar por el puerto.\n");
	}

	printf("Escuchando conexiones entrantes en el puerto %d.\n",port);

	return socketFD;

}

void actualizarDescriptorMaximo(int socketNuevaConexion,int* descr_max){
	if (socketNuevaConexion>*descr_max){
		*descr_max=socketNuevaConexion;
	}
}

void agregarNuevaConexionEnVectorClientesConectados(int socketNuevaConexion,int* vectorclientesconectados,int *tamaniovector){
	int i;
	for(i=0;i<*tamaniovector;i++){
		if(vectorclientesconectados[i]==0){
			vectorclientesconectados[i]=socketNuevaConexion;
			return;//si pude agregarlo salgo inmediatamente
		}
	}
	//si no pude agregarlo es por que no tengo lugar en el vector; entonces agrando el vector y luego lo agrego

}

void inicializarVectorEn0(int* vectorclientesconectados,int tamaniovector){
	int i;
	for(i=0;i<tamaniovector;i++){
			vectorclientesconectados[i]=0;
	}
}

void *threadPlanificador(void *parametro) {

		hplan* estructura=(hplan*)parametro;
		int tamanioVectorClientes=10;//seteo un valor inicial para tamanioVectorClientes
		int vectorClientesConectados[tamanioVectorClientes];//el vector puede crecer en tiempo de ejecucion, por lo que tamanioVectorClientes es un parametro inicial
	    fd_set readset, masterset;

	    int socketEscucha = sockets_create_Server(estructura->puerto);

	    //establecemos la conexion antes del while, esto es muy
	    //importante, por que es necesario agregar un descriptor al set
	    //de manera tal que detecte algo, sino queda bloqueado

		FD_ZERO(&readset);
		FD_ZERO(&masterset);//inicializamos en cero el set, esto hay que hacerlo 1 sola vez fuera del while. Te borra el contenido de los descriptores
					        //esto va dentro del while FD_SET(socketEscucha, &readset); //aca seteamos el descriptor de la conexion que aceptamos anteriormente  //********esto debe hacerse con el descriptor del socket que escucha, y guardar los descriptores de las nuevas conexiones en el set debe estar dentro del while

		inicializarVectorEn0(vectorClientesConectados,tamanioVectorClientes);
		int descr_max=socketEscucha; //descr max siempre tiene el nro mayor de socket (recordemos que el socket es un numero) dentro del masterset. Me da la impresion que el select internamente hace un for y por eso necesita que le pasemo el nro de socket maximo+1 por que es tipico en el for comparar de la manera i<max

		FD_SET(socketEscucha, &masterset);//fundamental para que el masterset arranque con el socket de escucha

		//agrego la conexion del nivel
		FD_SET(estructura->conexionNivel,&masterset);
		agregarNuevaConexionEnVectorClientesConectados(estructura->conexionNivel,vectorClientesConectados,&tamanioVectorClientes);
		actualizarDescriptorMaximo(estructura->conexionNivel,&descr_max);
		//fin agrego conexion nivel

		struct timeval TimeoutSelect; /* Ultimo parametro de select  */
		TimeoutSelect.tv_sec = 0;
	    TimeoutSelect.tv_usec = 100;

	    while(1){

	    	int result;

	    	while(1)
	    	{
	    	 	readset=masterset; //fundamental, para que el select pueda volver a monitorear todos los descriptores necesarios
	    		result= select(descr_max+1, &readset, NULL, NULL, &TimeoutSelect); // el cien es para evitar problemas. En varios codigos vi que siempre guardaban el valor del maximo
					   	   	   	   	   	   	   	   	   	   	   	   	   	       //  descriptor a medida que se iban generando las conexiones nuevas
				if ( !(result<0) || (result < 0 && errno !=EINTR) )
				{ //si hubo error vuelvo a realizar el select
					break;
				}
			}

			if (result > 0) //si encontro algun cambio en los descriptores
			{

				 if (FD_ISSET(socketEscucha, &readset)) //me fijo si hay nuevas conexiones mirando el descriptor del server
				 {
					//se agregan nuevas conexiones
					// aca se que son conexiones entrantes por que el cambio se produce en el socketescucha
					 int socketNuevaConexion;
					 socketNuevaConexion=accept(socketEscucha,0,0);
					 agregarNuevaConexionEnVectorClientesConectados(socketNuevaConexion,vectorClientesConectados,&tamanioVectorClientes);
					 FD_SET(socketNuevaConexion, &masterset);
					 actualizarDescriptorMaximo(socketNuevaConexion,&descr_max);
					 printf("se registro una nueva conexion\n");
				 }

				int i;

		        for (i=0; i<tamanioVectorClientes; i++) //recorro todos los descriptores de los clientes
				{

			       if ((vectorClientesConectados[i]!=0) && FD_ISSET(vectorClientesConectados[i], &readset)) //me fijo si alguno tiene datos en el descriptor
				   {

						//recibo datos de ese socket. si recibo 0 bytes significa que el cliente cerro la conexion y tengo que sacarlo del vectorClientesConectados
			    	    //luego proceso lo recibido y respondo o puedo esperar de recibir todo y despues responder todo junto.
			    	   char*buffer=malloc(50);
			    	   buffer[49]='\0';
			    	   if(recv(vectorClientesConectados[i],buffer,49,0)>0){
			    		   printf("%s\n",buffer);
			    		   send(vectorClientesConectados[i],buffer,50,0); //devuelvo lo que recibi. hago un simple eco para probar la conexion
			    	   }

				   }
				}
			}



	///aca puedo responder todo junto si acumule algo


	  } // cierro while(1)

  return NULL;

}
