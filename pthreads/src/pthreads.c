/*
 * pthreads.c
 *
 *  Created on: 09/04/2013
 *      Author: utnso
 */
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>

int sum; /* Este dato es compartido por los hilos */
void *runner(void *param); /*Hilo*/

int main(int argc, char *argv[])
{
	pthread_t tid; /* Identificador del hilo */
	pthread_attr_t attr; /*Setear los atributos del hilo*/

	if(argc!=2)
	{
		fprintf(stderr,"usage: a.out <integer value>\n");
		return -1;
	}
	if(atoi(argv[1])<0)
	{
		fprintf(stderr,"%d must be >=0\n",atoi(argv[1]));
		return -1;
	}
	/*Obtener los atributos por defecto*/
	pthread_attr_init(&attr);
	/*Crear el hilo*/
	pthread_create(&tid,&attr,runner,argv[1]);
	/*Esperar a que el hilo termine*/
	pthread_join(tid,NULL);

	printf("sum = %d\n",sum);
	return 0;
}

/* El hilo comenzará el control en esta función*/
void *runner(void *param)
{
	int i,upper=atoi(param);
	sum=0;

	for(i=1;i<=upper;i++)
		sum+=i;

	pthread_exit(0);
}
