/*
 * nivel.h
 *
 *  Created on: 09/05/2013
 *      Author: utnso
 */

#ifndef NIVEL_H_
#define NIVEL_H_
#define PERSONAJE_ITEM_TYPE 0
#define RECURSO_ITEM_TYPE 1

typedef struct item {
	char id;
	int posx;
	int posy;
	char item_type; // PERSONAJE o CAJA_DE_RECURSOS
	int quantity;
	struct item *next;
} ITEM_NIVEL;


int nivel_gui_dibujar(ITEM_NIVEL* items);
int nivel_gui_terminar(void);
int nivel_gui_inicializar(void);
int nivel_gui_get_area_nivel(int * rows, int * cols);


#endif /* NIVEL_H_ */
