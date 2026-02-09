
#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"

Entity* playerEntityNew(GFC_Vector2D position) 
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a player!");
	}

	self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;
	self->think = playerThink;
	self->velocity = gfc_vector2d(0,0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	//Entity thePlayer = self;
	//I have no fucking clue

	//self->think = playerThink();
	//self->free = playerFree();
	//self->update = playerUpdate();

	return self;

}
//think, update, free
void playerThink(Entity* self)
{
	
	if (!self)
		return;

	if (gfc_input_key_down("d"))
	{
		self->position.x += 1;

	}

	if (gfc_input_key_down("a"))
	{
		self->position.x -= 1;

	}

	if (gfc_input_key_down("s"))
	{
		self->position.y += 1;

	}

	if (gfc_input_key_down("w"))
	{
		self->position.y -= 1;

	}

	if (self->velocity.y || self->velocity.x)
	{		
		gfc_vector2d_normalize(&self->velocity);
		//gfc_vector2d_scale(self->velocity, self->velocity, self->topSpeed);
	}

}

void playerUpdate(Entity* self)
{

}

void playerFree(Entity* self)
{

}