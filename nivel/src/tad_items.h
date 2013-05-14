/*
 * tad_items.h
 *
 *  Created on: 09/05/2013
 *      Author: utnso
 */

#ifndef TAD_ITEMS_H_
#define TAD_ITEMS_H_

#include "src/nivel.h"

void BorrarItem(ITEM_NIVEL** i, char id);
void restarRecurso(ITEM_NIVEL* i, char id);
void MoverPersonaje(ITEM_NIVEL* i, char personaje, int x, int y);
void CrearPersonaje(ITEM_NIVEL** i, char id, int x , int y);
void CrearCaja(ITEM_NIVEL** i, char id, int x , int y, int cant);
void CrearItem(ITEM_NIVEL** i, char id, int x, int y, char tipo, int cant);

#endif /* TAD_ITEMS_H_ */
