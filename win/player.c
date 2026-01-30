
#include "simple_logger.h"
#include "entity.h"

Entity* playerEntityNew(GFC_Vector2D position) 
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
	}

	//self->sprite = gf2d_sprite_load_all();

}