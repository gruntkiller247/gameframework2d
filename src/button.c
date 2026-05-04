#include <SDL.h>
#include <stdio.h>
#include <simple_logger.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "button.h"
#include "UI.h"
#include "gf2d_graphics.h" 
#include "gf2d_draw.h"
#include "gfc_input.h"|
#include "entity.h"



typedef struct BD
{
	SDL_Texture* uiTexture;
	SDL_Rect*	uiRect;
}ButtonData;

void buttonFree(UI* self);
void changeLevel(UI* self);
void getOnClick(UI* ui, const char* onClick);
void spawn(UI* self);

UI* newButton(GFC_Vector2D position, GFC_Rect bounds, int type, const char* onClick)
{
	UI* ui = uiNew(position, bounds);

	if (!ui)
	{
		slog("Failed to make button UI!");
		return NULL;
	}

	ButtonData* data = malloc(sizeof(ButtonData));

	if (!data)
	{
		slog("Failed to allocate data for button!");
		return NULL;
	}

	data->uiTexture = NULL;

	getOnClick(ui, onClick);
	
	ui->type = type;
	

	ui->data = data;
	
	
	//ui->sprite = gf2d_sprite_load_image("images/ed210.png");
	//ui->onHoverSprite = gf2d_sprite_load_all("images/space_bug.png",128,128,16,0);

	
	ui->frame = 0;
	ui->rotation = 0;
	ui->active = 1;

	//slog("Active: %i", ui->active);
	
	ui->update = buttonUpdate;
	ui->touch = buttonTouch;
	ui->free = buttonFree;
	
	
	ui->clicked = 0;
	ui->hover = 0;

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
		//slog("Mouse is hovering over a UI element!");
		self->hover = 1;

		
		if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK && !self->clicked)
		{
			slog("Button has been clicked!");

			self->clicked = 1;
			if (self->onClick)
			{
				//self->clicked = 0;
				self->onClick(self);
			}	
			else
				slog("UI Element has no on click!");
		}
	}
	else
	{
		self->hover = 0;
		self->clicked = 0;
	}
}

void buttonUpdate(UI* self)
{
	if (!self)
		return;
	int temp = isPaused();
	//slog("THE UI THINKS THE GAME IS CURRENTLY: %i", temp);

	if (self->activeOnPause == 1 && isPaused() == PAUSED)
	{
		self->active = 1;
	}
	else if (self->activeOnPause == 0)
	{
		self->active = 1;
	}
	else
	{
		self->active = 0;
	}



	if (self->active != 1)
	{
		return;
	}

	ButtonData* data = (ButtonData*)self->data;

	if (!data)
		return;




	//self->frame++;
	if (self->frame >= 16)
		self->frame = 0;

	/*float x = self->position.x + self->bounds.x;
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
	gf2d_draw_line(BL, TL, GFC_COLOR_RED);*/



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

void unpause(UI* self)
{
	slog("Button trying to unpause!");
	setPausedUI();
}

void getOnClick(UI* ui, const char* onClick)
{
	slog("Checking the on Click!");

	if (!ui)
		return;

	if (!onClick)
		return;

	if (strcmp(onClick, "unpause") == 0)
	{
		//slog("Pause");
		ui->onClick = unpause;
	}
	else if (strcmp(onClick, "spawn") == 0)
	{
		//slog("spawn");
		ui->onClick = spawn;
	}
	else
	{
		slog("Null");
		ui->onClick = NULL;
	}

}

void spawn(UI* self)
{
	if (!self)
		return;
	
	if (isSpawning() == S_IS)
	{
		return;
	}
	else
	{
		slog("I am setting role to spawn %i", self->spawn);
		setUISpawning(self->spawn);
		slog("Stored as role: %i", roleToSpawn());
	}
	

	
}

/*
	Parses the onClick Function when read by JSON!
*/
void changeLevel(UI* self)
{
	slog("Inside CHange Lvel!");
}

void updateTexture(UI* self, SDL_Texture* texture)
{
	if (!self)
	{
		slog("No UI element to change texture on!");
		return;
	}
	ButtonData* data = (ButtonData*)self->data;
	
	if (!data)
	{
		slog("Error! UI Has no Data!");
		return;
	}

	if (!data->uiTexture)
	{
		data->uiTexture = texture;
		slog("Saved texture to UI Data!");
		return;
	}
	else
	{
		SDL_DestroyTexture(data->uiTexture);
		data->uiTexture = texture;
		return;
	}

	

}

SDL_Texture* getButtonTexture(UI* self)
{
	if (!self)
		return NULL;

	ButtonData* data = (ButtonData*)self->data;

	if (!data)
		return NULL;

	if (!data->uiTexture)
		return NULL;
	//slog("UI has texture!");
	return data->uiTexture;
}

