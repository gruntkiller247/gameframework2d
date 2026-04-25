#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "UI.h"
#include <SDL.h>
#include <stdio.h>
#include <simple_logger.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"

UI* newButton(GFC_Vector2D position, GFC_Rect bounds, int type, const char* onClick);

void buttonTouch(UI* self);

void buttonUpdate(UI* self);

void updateTexture(UI* self,SDL_Texture* texture);

SDL_Texture* getButtonTexture(UI* self);




#endif