#ifndef __UI_H__
#define __UI_H__

#include <SDL.h>
#include <stdio.h>
#include <simple_logger.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"

typedef struct UI_S
{
	Uint8 _inUse;	//Toggle whether UI elements needs to be destroyed
	Uint8 active;		//Toggle whether UI element is active on the screen
	GFC_Vector2D position;
	GFC_Rect bounds;
	int id;			//Unique ID for each element

	void* data;									//Data unique to the UI element
	//void (*think)(struct UI_S* self);			
	void (*update)(struct UI_S* self);
	void (*free)(struct UI_S* self);
	void (*touch)(struct UI_S* self);

	GFC_Color color;
	Sprite* sprite;
	int frame;
	GFC_Vector2D scale;
	int rotation;
	
}UI;

UI* uiNew(GFC_Vector2D position, GFC_Rect bounds);

void uiManagerInit(Uint32 max);

void uiManagerClose();

void uiTouchAll();

void uiUpdateAll();

void uiFreeAll();

void uiDrawAll();


//UI* uiNew(int width, int height, int posx, int posy);

/*
	Returns the UI element with the ID or NULL if not found
*/
UI* uiGetID(Uint32 id);


#pragma once
#endif