#include "simple_logger.h"
#include "projectiles.h"
#include "entity.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

Entity* projectileEntityNew(GFC_Vector2D position, float direction, Uint8 team)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a projectile!");
	}

	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = projectileThink;
	self->free = projectileFree;
	self->update = projectileUpdate;
	self->touch = projectileTouch;

	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(0, 0, 32, 32);

	self->team = team;
}


void projectileThink(Entity* self)
{
	if (!self)
		return;


}

void projectileTouch(Entity* self, Entity* toucher)
{
	if (!self)
		return;

	

	
}

void projectileUpdate(Entity* self)
{
	if (!self)
		return;

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

void projectileFree(Entity* self)
{
	if (!self)
		return;

	if (self->sprite)
		gf2d_sprite_free(self->sprite);

	free(self);
}