#include <SDL.h>
#include <stdio.h>
#include <simple_logger.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "button.h"
#include "UI.h"
#include "gf2d_graphics.h" 

typedef struct BD
{
	int temp;
}ButtonData;

void buttonFree(UI* self);

UI* newButton(GFC_Vector2D position, GFC_Rect bounds)
{
	UI* ui = uiNew(position, bounds);

	if (!ui)
	{
		slog("Failed to make button UI!");
		return NULL;
	}

	ButtonData* data = malloc(sizeof(ButtonData));

	ui->data = data;
	
	ui->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
	//ui->sprite = gf2d_sprite_load_image("images/ed210.png");

	if (!ui->sprite)
	{
		slog("Failed to load UI Button's Sprite!");
		return NULL;
	}

	ui->frame = 0;
	ui->rotation = 0;
	ui->active = 1;
	//slog("Active: %i", ui->active);
	
	ui->update = buttonUpdate;
	ui->touch = buttonTouch;
	ui->free = buttonFree;
	
	return ui;
}

void buttonTouch(UI* self)
{
	if (!self)
		return;

	if (self->active != 1)
		return;


	int mx;
	int my;

	SDL_GetMouseState(&mx, &my);

	if (mx >= self->position.x && mx <= self->position.x + self->bounds.w && my >= self->position.y && my <= self->position.y + self->bounds.h)
	{
		slog("Mouse is hovering over a UI element!");
	}
}

void buttonUpdate(UI* self)
{
	if (!self)
		return;

	if (self->active != 1)
		return;

	self->frame++;
	if (self->frame >= 16)
		self->frame = 0;

	float x = self->position.x + self->bounds.x;
	float y = self->position.y + self->bounds.y;
	float w = self->bounds.w;
	float h = self->bounds.h;

	GFC_Vector2D TL = gfc_vector2d(x, y);
	GFC_Vector2D TR = gfc_vector2d(x + w, y);
	GFC_Vector2D BR = gfc_vector2d(x + w, y + h);
	GFC_Vector2D BL = gfc_vector2d(x, y + h);

	gf2d_draw_line(TL, TR, GFC_COLOR_RED);
	gf2d_draw_line(TR, BR, GFC_COLOR_RED);
	gf2d_draw_line(BR, BL, GFC_COLOR_RED);
	gf2d_draw_line(BL, TL, GFC_COLOR_RED);

}

void buttonFree(UI* self)
{
	if (!self)
		return;

	if (self->sprite)
		gf2d_sprite_free(self->sprite);


	if (self->data)
	{
		free(self->data);
	}
		

	//free(self);

}
