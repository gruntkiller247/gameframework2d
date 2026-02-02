
#include "simple_logger.h"
#include "entity.h"
#include "player.h"

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

	//self->think = playerThink();
	//self->free = playerFree();
	//self->update = playerUpdate();

	return self;

}
//think, update, free
void playerThink(Entity* self)
{

}

void playerUpdate(Entity* self)
{

}

void playerFree(Entity* self)
{

}