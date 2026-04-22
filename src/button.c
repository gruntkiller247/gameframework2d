#include <SDL.h>
#include <stdio.h>
#include <simple_logger.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "button.h"
#include "UI.h"

typedef struct BD
{
	int temp;
}ButtonData;

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
		
}

void buttonTouch(UI* self)
{
	if (!self)
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

	self->frame++;
	if (self->frame >= 16)
		self->frame = 0;

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
		

	free(self);

}
