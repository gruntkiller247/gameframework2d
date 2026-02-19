
#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_shape.h"
#include "gfc_input.h"|
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "projectiles.h"



Entity* playerEntityNew(GFC_Vector2D position)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a player!");
	}


	strcpy(self->name,"Matt");

	self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = playerThink;
	self->free = playerFree;
	self->update = playerUpdate;
	self->touch = playerTouch;

	self->velocity = gfc_vector2d(0,0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(30, 30, 72, 72);

	self->team = 1;

	self->timeTillAttack = 5;
	
	self->lastShotRotation = 0;

	return self;

}



void playerThink(Entity* self)
{
	
	if (!self)
		return;



	if (gfc_input_key_down("z"))
	{
		//slog("Should be shooting a thing!");
		playerShoot(self);
		
	}

	if (gfc_input_key_down("d"))
	{
		self->position.x += 1;
		//self->rotation = 180;
		self->lastShotRotation = 180;

	}

	if (gfc_input_key_down("a"))
	{
		self->position.x -= 1;
		//self->rotation = 0;
		self->lastShotRotation = 0;

	}

	if (gfc_input_key_down("s"))
	{
		self->position.y += 1;
		//self->rotation = 270;
		self->lastShotRotation = 270;

	}

	if (gfc_input_key_down("w"))
	{
		self->position.y -= 1;
		//self->rotation = 90;
		self->lastShotRotation = 90;

	}

	if (self->velocity.y || self->velocity.x)
	{		
		gfc_vector2d_normalize(&self->velocity);
		//gfc_vector2d_scale(self->velocity, self->velocity, self->topSpeed);
	}

	//slog("Last shot rotation: %i", self->lastShotRotation);

}

void playerTouch(Entity* self, Entity* toucher)
{
	//Player Collision
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
		//slog("%s is touching something!",self->name);
	}

}

void playerUpdate(Entity* self)
{
	if (!self)
		return;

	//slog("Updateing Player!");

	self->frame += 0.1;

	if (self->frame >= 8)
		self->frame = 0;

	//Does not work for intended purpose, but makes a funny leash from world origin to player
	//gf2d_draw_line(gfc_vector2d(self->position.x, self->position.y), gfc_vector2d(self->bounds.x, self->bounds.y), GFC_COLOR_RED);

	//I hate writing code like this but debugging the wall of text made my migraine worse
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

void playerFree(Entity* self)
{
	if (!self)
		return;

	if (self->sprite)
		gf2d_sprite_free(self->sprite);

	free(self);
}

/*
	To be called in think during state fire
*/
void playerShoot(Entity* self)
{
	if (!self)
		return;

	//To DO fix the aiming
	Entity* thing = projectileEntityNew(self->position, self->lastShotRotation,TEAM_PLAYER, NULL);

	slog("INSIDER! Last shot rotation: %i", self->lastShotRotation);
	
	if (self->lastShotRotation = 180)
	{
		thing->velocity.x += 10;
		//180: right - d
		slog("Fireing Right!");
	}
	else if(self->lastShotRotation = 0)
	{
		thing->velocity.x -= 10;
		//0: left - a
		slog("Fireing Left!");
	}
	else if(self->lastShotRotation = 270)
	{
		thing->velocity.y -= 10;
		//270 down - s
		slog("Fireing down!");
	}
	else if (self->lastShotRotation = 90)
	{
		thing->velocity.y += 10;
		//90 up
		slog("Fireing Up!");
	}
	else
	{
		slog("Fireing No where!");
	}

}