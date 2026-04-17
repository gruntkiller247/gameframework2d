#include "simple_logger.h"
#include <simple_json.h>
#include "powerup.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

const char* powerUpFile = "JSONs/powerups.json";
void loadPowerUp(Entity* self);

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
	
	if(role == ROLE_PU_RANDOM)
		role = ROLE_PU_MIN+1 + rand() % (ROLE_PU_MAX-1-ROLE_PU_MIN);

	//Non JSON stuff
	self->position = position;
	self->frame = 0;
	self->think = powerUpThink;
	self->free = powerUpFree;
	self->update = powerUpUpdate;
	self->touch = powerUpTouch;

	self->team = TEAM_ITEM;
	self->layer = EL_ITEM;
	self->role = role;
	self->bounds = gfc_rect(0, 0, 32, 32);

	//strcpy(self->name, role);

	
	loadPowerUp(self);
	
	

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

void loadPowerUp(Entity* self)
{

	if (!self)
		return;

	if (!powerUpFile)
	{
		slog("Error finding powerup JSON file!");
		return;
	}

	SJson* json = NULL;
	SJson* rson = NULL;
	SJson* roleData = NULL;
	const char* spriteFile = NULL;
	const char* colorReal = NULL;
	int ultPowerUp = -1;
	int powerUpMaxTime = -1;

	json = sj_load(powerUpFile);

	if (!json)
	{
		slog("Failed to load powerup JSON file!");
		goto fail;
	}

	spriteFile = sj_object_get_string(json, "sprite");

	if (!spriteFile)
	{
		slog("powerup has no sprite!");
		goto fail;
	}

	self->sprite = gf2d_sprite_load_all(spriteFile, 128, 128, 16, 0);
	
	if (sj_object_get_int(json, "powerUpMaxTime", &powerUpMaxTime) == 0)
	{
		slog("Failed to get powerupMaxTime or it is 0!");
		powerUpMaxTime = 0;
	}
	
	self->powerUpMaxTime = powerUpMaxTime;


	rson = sj_object_get_value(json, "roles");

	if (!rson)
	{
		slog("Failed to load JSON role array!");
		goto fail;
	}
	

	//Role stuff
	switch (self->role)
	{
	case ROLE_PU_FREE_ULT:
		roleData = sj_array_get_nth(rson, 0);
		colorReal = sj_object_get_string(roleData, "color");
		
		if (getColor(self, colorReal) == 0)
		{
			//Do the real color comparison here
			//This means the JSON used the format of gfc_color(X,Y,Z);
		}

		if (sj_object_get_int(json, "powerUpMaxTime", &powerUpMaxTime) == 0)
		{
			slog("Failed to get ultPowerup from JSON!");
			goto fail;
		}

		self->powerUpMaxTime = powerUpMaxTime;
		break;

	case ROLE_PU_INVUL:
		roleData = sj_array_get_nth(rson, 1);
		colorReal = sj_object_get_string(roleData, "color");

		if (getColor(self, colorReal) == 0)
		{
			//Do the real color comparison here
			//This means the JSON used the format of gfc_color(X,Y,Z);
		}

		break;

	case ROLE_PU_HP_RECOVERY:
		roleData = sj_array_get_nth(rson, 2);
		colorReal = sj_object_get_string(roleData, "color");

		if (getColor(self, colorReal) == 0)
		{
			//Do the real color comparison here
			//This means the JSON used the format of gfc_color(X,Y,Z);
		}
		break;

	case ROLE_PU_SPEED:
		roleData = sj_array_get_nth(rson, 3);
		colorReal = sj_object_get_string(roleData, "color");
		;

		if (getColor(self, colorReal) == 0)
		{
			//Do the real color comparison here
			//This means the JSON used the format of gfc_color(X,Y,Z);
		}

		if (sj_object_get_int(json, "powerUpMaxTime", &powerUpMaxTime) == 0)
		{
			slog("Failed to get ultPowerup from JSON!");
			goto fail;
		}

		self->powerUpMaxTime = powerUpMaxTime;
		break;

	case ROLE_PU_BOMB:

		break;

	default:
		slog("Error with spawning a powerup!");
		return NULL;

	}

	fail:
	
	if (json)
		sj_free(json);
}