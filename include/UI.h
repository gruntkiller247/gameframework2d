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
	Uint8 _inUse;								//Toggle whether UI elements needs to be destroyed
	Uint8 active;								//Toggle whether UI element is on the screen
	Uint8 activeOnPause;						//Toggle whether the UI should be shown when the game is paused
												//1 means the UI should be shown ONLY WHEN THE GAME IS PAUSED
												//0 means it is always active
	GFC_Vector2D position;
	GFC_Rect bounds;
	int id;										//Unique ID for each element

	void* data;									//Data unique to the UI element		
	void (*update)(struct UI_S* self);
	void (*free)(struct UI_S* self);
	void (*touch)(struct UI_S* self);
	void (*onClick)(struct UI_S* self);

	GFC_Vector2D scale;
	GFC_Color color;
	Sprite* sprite;
	Sprite* onHoverSprite;
	Sprite* onClickSprite;
	
	int rotation;
	int frame;
	Uint8 hover;								//Bool indicating whether a UI is being hovered over by the mouse
	Uint8 clicked;								//Bool indicating whether a UI element has been clicked!
	Uint8 type;									//What UI type is

	
}UI;

typedef enum
{
	UI_ERROR = 0,
	UI_BUTTON = 1,
	UI_IMAGE,
	UI_THING
}UI_TYPE;

typedef enum
{
    PAUSED=0,
    NOT_PAUSED=1
}PausedENUMS;

UI* uiNew(GFC_Vector2D position, GFC_Rect bounds);

void uiManagerInit(Uint32 max);

void uiManagerClose();

void uiTouchAll();

void uiUpdateAll();

void uiFreeAll();

void uiDrawAll();

void uiKillAll();


//UI* uiNew(int width, int height, int posx, int posy);

/*
	Returns the UI element with the ID or NULL if not found
*/
UI* uiGetID(Uint32 id);

/*
	Flips the bool for isPaused
*/
void setPausedUI();

/*
	Helper for children UI. Returns if the game is paused.
	0: Paused. 1: the game is running
*/
int isPaused();


#pragma once
#endif