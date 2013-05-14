/*
 ============================================================================
 Name        : personaje.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
//-----------------------------------------------------------------------------
#define PATH_CONFIG "/home/utnso/git/tp-20131c-so-pa/personaje/archivos_config"
#define BUFF_SIZE 1024
#define MAX_NIVELES 10
#define IP_ORQUESTADOR "127.0.0.1"
#define PUERTO_ORQUESTADOR 15000
//-----------------------------------------------------------------------------
// Definicion de la estructura que obtiene los datos del archivo de configuracion

typedef struct data{
	char *clave;
	char *valor;
} dato;

typedef struct coordenadas{
	int posX;
	int posY;
} coordenadas;


typedef struct nodo {
	void* data;
	struct nodo *next;
}nodo;

// estructura para la variable local niveles, va a guardar en un campo el nivel y
//en el otro un puntero a una lista con los objetivos de ese nivel
typedef struct camposNivel{
	char* niveles;
	nodo* ptrObjetivosNivel;
}camposNivel;

struct pndata{
	char ipPlanificador[16];
	int puertoPlanificador;
	char ipNivel[16];
	int puertoNivel;
} __attribute__((packed));

typedef struct pndata pndata;

//------------------------------------------------------------------------------------
// Variables Globales
camposNivel niveles[MAX_NIVELES];
int vidas;
char* ip;
char* puerto;
char* nombre;
char* simbolo;
int socketConexionOrquestador;
int nivelActual; //indica la posicion del vector niveles en la cual estoy trabajando
int posX,posY;   //indican las coordenadas X e Y de la pantalla de movimientos en donde el personaje esta parado actualmente
//-------------------------------------------------------------------------------
//   Prototipos de funciones

int notificarBloqueoPlanificador(int socketPlanificador,char *objetivo);
void notificarMovimientoAlNivel(int posicionX,int posicionY,int socketConexionNivel);
int pedirEntregaRecurso(int socketNivel,char *objetivo);
coordenadas pedirPosSiguienteObjetivo(int socketNivel,char *recurso);
void conectarConNivel(char* ip, int puerto);
int obtenerOrquestardorDePlataforma();
int contarNodos(nodo *lista);
int comparar3primeros(char *str1,const char *str2);
void parsearObjetivos(nodo *lista);
void* mostrarListaVector(nodo *lista,void(*func)(dato*));
void mostrarNodoChar(void*data);
void funcion(FILE*,nodo**);
void separarPorIgual(char*,char**,char**);
void crearNodo(nodo**,void*);
void* mostrarLista(nodo*,void(*)(dato*));
void mostrarStruct(dato*);
void eliminarLista(nodo**);
int sockets_create_Client(char *ip, int port);
void sockets_send(int socket);
void buscarEnLista(nodo*,char*);
void parsearNivel (nodo* lista, char* data,int);
void inicializarVector(camposNivel vec[],int max);
void mostrarVector(camposNivel vector[],int max);
int comparar(char*,const char*);
void completarVariablesGlobales(nodo*);
void eliminarListas(nodo* lista);
void cargarTodasLasVariablesGlobales();
int devolverNivelConCero(char*,int,char*,int);
void modificarNivelEnVector();
void jugarNivel(pndata*);
void jugarTodosLosNiveles();
pndata* obtenerEstructura();
//void conectarPersonaje(pndata* data);
int pedirAutorizacionMovimiento(int);
void funcionMover(int posicionRecursoX,int posicionRecursoY,int,int);

int main() {

	cargarTodasLasVariablesGlobales();
	jugarTodosLosNiveles();

	//esperar para jugar koopa

	return 0;
}

void funcion(FILE *arch,nodo **lista)
{ /*esta funcion se encarga de meter los datos del archivo de configuracion en una lista
    separandolos por clave y valor ej: nombre(clave) mario(valor) , cada linea del
    archivo es un nodo*/

	dato* dato;
	char buffer[50];
	char* claveArch;
	char* valorArch;

	//buffer=malloc(sizeof(char*));
	claveArch=malloc(sizeof(char*));
	valorArch=malloc(sizeof(char*));

	dato=malloc(sizeof(dato));

    fgets(buffer,50,arch);

	while(!(feof(arch)))
	{
		dato=malloc(sizeof(dato));
		separarPorIgual(buffer,&claveArch,&valorArch);
		dato->clave=claveArch;
		char *ptr=valorArch;
		while(*ptr!='\n')
		{ptr++;}
		*ptr='\0';
		dato->valor=valorArch;
		crearNodo(lista,dato);
		fgets(buffer,50,arch);
	}
}

void separarPorIgual(char* buffer,char **clave,char **valor)
{ //esta funcion guarda en clave y valor estos valores luego de ser parseados
	(*clave)=strdup(strtok(buffer,"="));
	(*valor)=strdup(strtok(NULL,"="));

}

void crearNodo(nodo **lista,void *dato){
	//crea un nodo para la lista de lineas parseadas en clave, valor
	nodo *aux;
	if(*lista==NULL){
			*lista=malloc(sizeof(nodo));
			(*lista)->data=dato;
			(*lista)->next=NULL;
	}else{
			aux=*lista;

			while(aux->next!=NULL){
				aux=aux->next;
			}

			(aux->next)=malloc(sizeof(nodo));
			(aux->next)->data=dato;
			(aux->next)->next=NULL;
	}
}

void* mostrarLista(nodo *lista,void(*func)(dato*)){
	//muestra la lista del archivo parseado
nodo *aux;

if(lista==NULL){
	printf("lista vacia. Presione cualquier letra para salir");
	getchar();
	exit(0);
}
		aux=lista;
		while(aux!=NULL){
			(*func)(aux->data);
			aux=aux->next;
		}
		//printf("lista mostrada. presione una tecla para continuar...\n");
		//getch();
		return NULL;
}

void* mostrarListaVector(nodo *lista,void(*func)(dato*)){
	//muestra la lista que esta contenida en el vector niveles
	nodo *aux;

	if(lista==NULL){
		printf("objetivos vacios\n");

		return NULL;
	}
			aux=lista;
			while(aux!=NULL){
				(*func)(aux->data);
				aux=aux->next;
			}
			//printf("lista mostrada. presione una tecla para continuar...\n");
			//getch();
			return NULL;
	}

void mostrarStruct(dato* data)
{ //muestra el campo donde se guarda el nivel en niveles
	printf("%s %s",data->clave,data->valor);
}

void eliminarLista(nodo **lista){
//elimina nodos de una lista
	nodo *aux;

	if(*lista==NULL){
		printf("Advertencia al querer liberar la memoria de la lista. La lista se encontro vacia.\nPresione cualquier letra para salir");
		//getchar();
		exit(0);
	}
	aux=*lista;
	while(aux!=NULL){
				aux=aux->next;
				//free((*lista)->data);
				free(*lista);
				*lista=aux;
	}
}

void parsearNivel (nodo* lista, char* data,int j)
{ /*recibe la lista del archivo de configuracion, el campo buscado para hacer el parseo
y como tercer parametro un entero que sirve para moverse en el vector para el caso de insertar objetivos
esta funcion tiene dos funcionalidades:
1-parsea los niveles obtenidos del campo valor del nodo que tiene el plan de niveles y los inserta en un vector
2-parsea los objetivos por nivel de cada nivel y los guarda en el vector niveles
	*/
	char* pch;
	char* claveNodo;
	nodo *ptr;
	ptr=lista;
	int encontrado=0;
	//posiciona el puntero sobre el nodo que en la clave tiene el valor buscado
	while((ptr!=NULL)&&(encontrado==0))
	{
		claveNodo=((dato*)(ptr->data))->clave;
		if(comparar(claveNodo,data))
			encontrado=1;
		else ptr=(ptr->next);
	}
	if(encontrado==1)
	{//si se encontro el nodo que tiene el plan de niveles o los objetivos por nivel, los separo
		pch=strtok(((dato*)(ptr->data))->valor,"[],");
		int i=0;
		while ((pch != NULL)&&(*pch!='\n'))
		  { //pregunto si me interesa parsear e insertar los niveles o los objetivos por nivel
			if(comparar(claveNodo,"planDeNiveles")){
		    niveles[i].niveles=pch;
		    i++;
		    }
			else{
				crearNodo(&niveles[j].ptrObjetivosNivel,pch);

						    }

			pch = strtok (NULL,"[],");
			}

		  }
	}

void parsearObjetivos(nodo *lista){
	//es la encargada de recorrer los campos de objetivos y mandarselos a la funcion
	//parsear nivel para que haga lo suyo
	char *clave;
	nodo *ptr=lista;
	int posObjetivosEnVector=0;
	clave=((dato*)(lista->data))->clave;
	while(ptr!=NULL){
		clave=((dato*)(ptr->data))->clave;
		if(comparar3primeros(clave,"obj")){
		parsearNivel(lista,clave,posObjetivosEnVector);
		posObjetivosEnVector++;
		}
		ptr=(ptr->next);
	}

}

void inicializarVector(camposNivel vec[],int max)
{ int i;
//inicializa el vector
	for(i=0;i<max;i++)
		{
		vec[i].niveles=NULL;
		vec[i].ptrObjetivosNivel=NULL;//malloc(sizeof(nodo));
		}
}

void mostrarVector(camposNivel vector[],int max)
{ //muestra el vector
	int i;
	for(i=0;i<max;i++)
		{
			if(vector[i].niveles!=NULL)
			{
				printf("%s\n",vector[i].niveles);
				printf("los objetivos son:\n");
				mostrarListaVector(vector[i].ptrObjetivosNivel,(void*)mostrarNodoChar);
				printf("\n");
			}
		}
}
int comparar(char *str1,const char *str2)
{//compara si dos cadenas de caracteres son iguales
	return !(strncmp(str1,str2,strlen(str2)));
}

void mostrarNodoChar(void*data){
	//muestra el char que contiene un nodo
	printf("%c",*((char*)data));
}

int comparar3primeros(char *str1,const char *str2){
	//compara si la primer cadena es igual en los 3 primeros caracteres a la segunda
	//sirve para identificar a los objetivos por nivel
	int i;
	int igual=0;
	for(i=0;i<3;i++){
		if(*str1==*str2){
			igual=1;
			str1++;
			str2++;
		}
		else {igual=0;
		i=3;
		}
	}
	return igual;
}

void completarVariablesGlobales(nodo *lista){
	nodo *ptr=lista;
	char *claveNodo;
	nombre=(char *)malloc(sizeof(char*));
	simbolo=malloc(sizeof(char*));
	nivelActual=0;
	posX=0;
	posY=0;
	while(ptr!=NULL){
		claveNodo=(((dato*)(ptr->data))->clave);
		if(comparar(claveNodo,"nombre"))
		{ nombre=(((dato*)(ptr->data))->valor);}	else{
				if(comparar(claveNodo,"simbolo"))
					{simbolo=(((dato*)(ptr->data))->valor);}	else{
						if(comparar(claveNodo,"vidas"))
							{vidas=atoi(((dato*)(ptr->data))->valor);}



					         }
			   }
		ptr=ptr->next;}

		}

void buscarEnLista(nodo* lista,char* dato1 )
{
 nodo* aux;
 aux=lista;
 puerto=(char*)malloc(sizeof(char*));
 ip=(char *)malloc(sizeof(char*));
 while(aux!=NULL)
 {
  if(!strcmp((((dato*)(aux->data))->clave),dato1))
  {
   ip=strdup(strtok(((dato*)(aux->data))->valor,":"));
   puerto=(char*)strdup(strtok(NULL,"\0"));
   aux=aux->next;
  }
  else
  {
   aux=aux->next;
  }
 }
}

void eliminarListas(nodo* lista){
	eliminarLista(&lista);
		/*int i=0;
		for(i=0;i<MAX_NIVELES;i++){
			if((niveles[i].niveles!=NULL)&&(*niveles[i].niveles!='\n'))
			eliminarLista(&(niveles[i].ptrObjetivosNivel));
		}*/

}
void cargarTodasLasVariablesGlobales(){
	nodo *lista=NULL;
	FILE *arch_config=fopen(PATH_CONFIG,"r");
	//--------Se parsea el archivo de configuracion
		funcion(arch_config,&lista);
		fclose(arch_config);
		inicializarVector(niveles,MAX_NIVELES);
	//-----Se separan los niveles que estaban todos en un mismo campo en la lista
		parsearNivel(lista,"planDeNiveles",0);
	//-----Se separan los objetivos que estaban todos en un solo campo en la lista y se los inserta en el vector
		parsearObjetivos(lista);
	//-----Se separan el puerto y la ip que estaban en el mismo campo en la lista
			buscarEnLista(lista,"orquestador");
	//-----Se completan las variables globales que faltan nombre simbolo y vidas
			completarVariablesGlobales(lista);
			modificarNivelEnVector();


			/*------Muestra la lista que contiene los datos del archivo de configuracion
				se utiliza solo como checkeo

				mostrarLista(lista,mostrarStruct);
				*/


				//---- Muestra los datos que se cargaron en el vector se usa para checkeo

				mostrarVector(niveles,MAX_NIVELES);

				eliminarListas(lista);

}

int devolverNivelConCero(char* nivel,int longitud,char* buffer,int tamaniobuffer)
{
/*
  1° parametro: cadena de caracteres a rellenar
  2° parametro: longitud total de la cadena luego de ser rellenada, sin contar el '\0' al final
  3° parametro: buffer donde almacenar el resultado
  4° parametro: tamaño del buffer para validar que no se pase del rango. El tamanio del buffer debe incluir el espacio para el '/0'

  retorna -1 en caso de error.
  retorna 0 en caso de exito.
*/

	int i;

	if (longitud<strlen(nivel))
	{
		printf("error: Longitud incorrecta\n");
		return -1;
	}
	if(longitud>=tamaniobuffer)
	{
		printf("error: el tamaño del buffer no alcanza\n");
		return -1;
	}
	for(i=0;i<(longitud-strlen(nivel));i++)
	{
		strcat(buffer,"0");
	}
	strcat(buffer,nivel);
	return 0;
}

void modificarNivelEnVector(){
	int tamanioBufer=3+1;
	char numeroRellenado[tamanioBufer];
	char *cadenaFinal;
	char *ptr=malloc(sizeof(char*));

	int i;
	for(i=0;i<MAX_NIVELES;i++){
		if((niveles[i].niveles)!=NULL){
			numeroRellenado[0]='\0';
			cadenaFinal=malloc(sizeof(char*));
			cadenaFinal="NIVEL ";
			ptr=malloc(sizeof(char*));
			*ptr='\0';

			devolverNivelConCero((niveles[i].niveles)+5,3,numeroRellenado,tamanioBufer);	//strcat(cadenaFinal,numeroRellenado);

			(niveles[i].niveles)=strcat(strcat(ptr,cadenaFinal),numeroRellenado);

		}

	}

}

void jugarNivel(pndata *data){
//	el personaje se va a ir moviendo para conseguir los recursos de su objetivo
//
//	--primero el personaje se conecta a el nivel y al planificador---------------

	//me conecto al nivel y al planificador
	int socketPlanificador,socketNivel;
		socketPlanificador = sockets_create_Client(data->ipPlanificador,data->puertoPlanificador);
		printf("me conecte al planificador\n");
		socketNivel = sockets_create_Client(data->ipNivel,data->puertoNivel);
		printf("me conecte al planif y al nivel\n");
	//-----------------------------------------------------------------------------
	//creo una lista para saber los recursos ya obtenidos
	nodo* listaObjetivosCompletados=malloc(sizeof(nodo*));
	nodo *auxObjetivosNivel=niveles[nivelActual].ptrObjetivosNivel;
	char *objetivo=malloc(sizeof(char)*2);

	coordenadas posicion;
	int a,b;
	a=b=0;
	//si la lista de objetivos totales que esta en el vector no tiene la misma cantidad que la de objetivos completados sigo buscando nuevos recursos
	while((a=(contarNodos(listaObjetivosCompletados)))!=(b=(contarNodos(niveles[nivelActual].ptrObjetivosNivel)+1))){
			strcpy(objetivo,((char*)(auxObjetivosNivel->data)));
			 printf("pido otro recurso %s\n",objetivo);
			posicion=pedirPosSiguienteObjetivo(socketNivel,objetivo);
			funcionMover(posicion.posX,posicion.posY,socketNivel,socketPlanificador);//mueve al personaje cuando el planificador se lo permite, modifica la posicion del mismo, le avisa al planificador que realizo un movimiento, le avisa al nivel para que lo grafique, todo esto paso por paso hasta que llega al recurso



	 if(pedirEntregaRecurso(socketNivel,objetivo)){//se le pide al nivel que entregue el recurso y actualice su stock de recursos y si me lo entrega devuelve 1, sino devuelve 0
		 pedirAutorizacionMovimiento(socketPlanificador);//le aviso al planificador que obtuve el recurso para que me consuma 1 de quantum
	 }
	 else {
		 notificarBloqueoPlanificador(socketPlanificador,objetivo);//le digo al planificador que me ponga en la cola de bloqueados de ese recurso y me quedo esperando hasta que el planificador me diga que lo tengo disponible
	 }
	 crearNodo(&listaObjetivosCompletados,objetivo);//inserto el objetivo obtenido en la lista de objetivos completados
	 auxObjetivosNivel=auxObjetivosNivel->next; //busco el prox objetivo



	}
}

void jugarTodosLosNiveles(){
	int socketOrquestador;
	socketOrquestador=obtenerOrquestardorDePlataforma();
	for(;nivelActual<MAX_NIVELES;nivelActual++)
	{
			if(niveles[nivelActual].niveles!=NULL)
			{
				pndata *puertosIp = obtenerEstructura();//le pide al orquestador el ip y puerto del planificador y los niveles
				jugarNivel(puertosIp);
				printf("termine el nivel\n");
			}
	}
}

int contarNodos(nodo *lista){
	int cantidad=0;
	while(lista!=NULL){
		cantidad++;
		lista=lista->next;
	}
	return cantidad;
}

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

	while(connect(socketFD, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!=0)
	{
		sleep(1000);
	}

	return socketFD;
}


void sockets_send(int cliente)
{
	int bytesEnviados;
		void *buffer4=malloc(40);
		bytesEnviados = send(cliente,niveles[nivelActual].niveles,10,0);

		if (bytesEnviados < 0)
			perror("Error al enviar datos");
			recv(cliente, buffer4,40, 0);
			printf("el ipPlanificador es: %s\n",((pndata*) buffer4)->ipPlanificador);
			printf("el ipNivel es: %s\n",((pndata*) buffer4)->ipNivel);
			printf("el puertoPlanificador es: %d\n",((pndata*) buffer4)->puertoPlanificador);
			printf("el puertoNivel es: %d\n",((pndata*) buffer4)->puertoNivel);
			close(cliente);
			conectarConNivel(((pndata*) buffer4)->ipNivel,((pndata*) buffer4)->puertoNivel);
}

pndata* obtenerEstructura(){

	int socketConexionOrquestador;
	pndata *buffer=malloc(sizeof(pndata));

	if (send(socketConexionOrquestador=sockets_create_Client(ip,atoi(puerto)),niveles[nivelActual].niveles,10,0) < 0)//La variable niveles[nivelActual] hace referencia al nivel del q se quieren obtener los datos
	{
		perror("Error al enviar datos");
	}

		recv(socketConexionOrquestador, buffer,40, 0);
		printf("recibi las cosas del orquestador\n");
		close(socketConexionOrquestador);

//		printf("el ipPlanificador es: %s\n",((pndata*) buffer4)->ipPlanificador);
//		printf("el ipNivel es: %s\n",((pndata*) buffer4)->ipNivel);
//		printf("el puertoPlanificador es: %d\n",((pndata*) buffer4)->puertoPlanificador);
//		printf("el puertoNivel es: %d\n",((pndata*) buffer4)->puertoNivel);

		return buffer;
}

void conectarConNivel(char* ip, int puerto)
{
	int unSocket;
	char buffer[20];

	unSocket  = sockets_create_Client(ip,puerto);
	send(unSocket,"hora",5,0);
	recv(unSocket,buffer,20,0);
	printf("%s\n",buffer);
}

/*void conectarPersonaje(pndata* data)
{
	//int socketPlanificador,socketNivel;
	int socketPlanificador=sockets_create_Client(data->ipPlanificador,data->puertoPlanificador);
	int socketNivel=sockets_create_Client(data->ipNivel,data->puertoNivel);
}*/


int pedirAutorizacionMovimiento(int socketConexionPlanificador)
{
	//esta función devuelve 1 si el personaje tiene autorizacion para moverse
	//y 0 si no tiene autorizacion. Suponiendo q estamos conectados al planificador
	//hacer un send con el comando para pedir autorizacion
	//me bloqueo esperando recibir la respuesta

	char buffer[10];
	while(1)
	{
		if(send(socketConexionPlanificador,"AUMOV",5,0)>0)
		{
			if(recv(socketConexionPlanificador,buffer,5,0)>0)
			{
				if(comparar(buffer,"OK"))
				{
					printf("me autorizaron el movimiento\n");
					return 1;

				}

			}else{

				printf("Error al recibir los datos");
				return -1;
			}

		}else{

			printf("error al enviar los datos");
			return -2;

		}
	}
return -3;}

void funcionMover(int posicionRecursoX,int posicionRecursoY,int socketNivel,int socketPlanificador)
{
	//La posicion del personaje está en variables globales
	while((posX!=posicionRecursoX) | (posY!=posicionRecursoY))
	{
		if(posX<posicionRecursoX )
		{
				if(pedirAutorizacionMovimiento(socketPlanificador)){
				posX++;}
		}else if(posX>posicionRecursoX ){
			if(pedirAutorizacionMovimiento(socketPlanificador)){
			posX--;}
		}

		else if(posY<posicionRecursoY )
		{
			if(pedirAutorizacionMovimiento(socketPlanificador)){
			posY++;}
		}else if(posY>posicionRecursoY ){
			if(pedirAutorizacionMovimiento(socketPlanificador)){
			posY--;}
		}
		printf("pos actual %d %d\n",posX,posY);
		notificarMovimientoAlNivel(posX,posY,socketNivel);
	}
	//Se pediran instancias del recurso al planificador del nivel y si no hay recursos disponibles se bloquea
}



void notificarMovimientoAlNivel(int posicionX,int posicionY,int socketConexionNivel)
{
 char buffer[13];
 //mandar una estructura coordenada
 coordenadas bufferEstructura;
 bufferEstructura.posX=posicionX;
 bufferEstructura.posY=posicionY;
 memcpy(buffer,"PPMOV",5);
 memcpy(buffer+5,&bufferEstructura,sizeof(coordenadas));
 send(socketConexionNivel,buffer,13,0);


}

coordenadas pedirPosSiguienteObjetivo(int socketNivel,char *recurso){
	// posRe[caracter recurso] es el mensaje que recibe el nivel para devolver la posicion del recurso solicitado
	coordenadas buffer;
	char *mensaje=malloc(sizeof(char)*7);
	mensaje=strcat(strdup("POSRE"),recurso);

		if (send(socketNivel,mensaje,6,0) < 0)
		{
			perror("Error al enviar datos");
		}

			recv(socketNivel, &buffer,8, 0);
			printf("el recurso esta ubicado en la posicion %d %d\n",buffer.posX,buffer.posY);
			//close(socketNivel);

			return buffer;
}

int pedirEntregaRecurso(int socketNivel,char *objetivo){

	// objetivo debe ser pasado como una constante de char *
	char buffer[3];//recibo OK
	char *mensaje=malloc(sizeof(char)*7);
	mensaje=strcat(strdup("ENTRE"),objetivo);
	printf("entre prox recurso: %s\n",objetivo);

		if (send(socketNivel,mensaje,6,0) < 0)
		{
			perror("Error al enviar datos");
		}

			recv(socketNivel, buffer,3, 0);
			//close(socketNivel);
			if(comparar(buffer,"OK")){
				return 1;
			}else{
				return 0;
			}

}
int notificarBloqueoPlanificador(int socketPlanificador,char *objetivo){
	//hacer un send al planificador para que me ponga en la lista de bloqueados y esperar en un recv el desbloqueo
	char buffer[10];
	char *mensaje=malloc(sizeof(char)*7);
		mensaje=strcat(strdup("BLOQR"),objetivo);
		while(1)
		{
			if(send(socketPlanificador,mensaje,7,0)>0)
			{
				if(recv(socketPlanificador,buffer,5,0)>0)
				{
					if(comparar(buffer,"OK"))
					{
						return 1;

					}else if(comparar(buffer,"NOAUT"))
					 {
						return 0;
					 }

				}else{

					printf("Error al recibir los datos");
					return -1;
				}

			}else{

				printf("error al enviar los datos");
				return -2;

			}
		}
	return -3;
}
int obtenerOrquestardorDePlataforma(){
	//falta definir esta funcion
	return 1;}






