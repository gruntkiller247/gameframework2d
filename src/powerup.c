#include "simple_logger.h"
#include "powerup.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

void powerUpThink(Entity* self);

void powerUpTouch(Entity* self, Entity* toucher);

void powerUpUpdate(Entity* self);

void powerUpFree(Entity* self);

Entity* powerUpEntityNew(GFC_Vector2D position,int role)
{
	Entity* self;
	int num;

	self = entityNew();

	if (!self)
	{
		return NULL;
	}

	if(role == PU_RANDOM)
		role = rand() % PU_MAXNUMBER;

	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = powerUpThink;
	self->free = powerUpFree;
	self->update = powerUpUpdate;
	self->touch = powerUpTouch;

	self->bounds = gfc_rect(0, 0, 32, 32);
	
	self->team = TEAM_ITEM;
	self->layer = EL_ITEM;
	self->role = role;

	//TO DO: Add all power ups + functionality
	//Add color based on powerup!
	switch (role)
	{
		case PU_FREE_ULT:
			self->colorReal = GFC_COLOR_GREY;
			self->ultPowerup = 1;
			break;

		case PU_INVUL:
			self->colorReal = GFC_COLOR_BLUE;

			break;

		case PU_HP_RECOVERY:
			self->colorReal = GFC_COLOR_RED;
			break;

		case PU_SPEED:
			//self->powerUpTimer = 0;
			self->colorReal = GFC_COLOR_CYAN;
			self->powerUpMaxTime = 400;
			break;

		case PU_BOMB:
			
			break;

		default:
			slog("Error with spawning a powerup!");
			return NULL;
			
	}

	return self;
}


void powerUpThink(Entity* self)
{
	if (!self)
		return;

}

void powerUpTouch(Entity* self, Entity* toucher)
{
	if (!self || !toucher)
		return;

	float selfLeft = self->position.x + self->bounds.x;
	float selfRight = selfLeft + self->bounds.w;
	float selfTop = self->position.y + self->bounds.y;
	float selfBottom = selfTop + self->bounds.h;

	float toucherLeft = toucher->position.x + toucher->bounds.x;
	float toucherRight = toucherLeft + toucher->bounds.w;
	float toucherTop = toucher->position.y + toucher->bounds.y;
	float toucherBottom = toucherTop + toucher->bounds.h;

	if (selfLeft < toucherRight && selfRight > toucherLeft && selfTop  < toucherBottom && selfBottom > toucherTop)
	{
		//slog("%s is touching something! %s",self->name,toucher->name);
	}
}

void powerUpUpdate(Entity* self)
{
	if (!self)
		return;

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

void powerUpFree(Entity* self)
{
	if (!self)
		return;

	if (self->sprite)
	{
		gf2d_sprite_free(self->sprite);
	}

	if (self->data)
		free(self->data);

	free(self);
}
