#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"


typedef enum
{
	MS_Idle,
	MS_Hunt,
	MS_Attack,
	MS_Pain,
	MS_Die,
	MS_MAX
}MonsterStates;

typedef struct MD
{
	Entity* player;
	MonsterStates state;
}MonsterData;



void monsterTouch(Entity* self)
{
	if (!self)
		return;
}

void monsterUpdate(Entity* self)
{
	if (!self)
		return;

	self->frame += 0.1;
	if (self->frame >= 8)
		self->frame = 0;
}

void monsterThink(Entity* self)
{
	//GFC_Vector2D toPlayer = { 0 };
	//MonsterData *data;

	if (!self || !self->data)
		return;

	slog("Monster is thinking!");

	//I have no fucking clue
	//gfc_vector2d_sub(toPlayer)
}

void monsterFree(Entity* self)
{
	MonsterData* data;

	if (!self)
		return;

	data = self->data;
	//clean up anything I own

	free(data);

	if (self->sprite)
		gf2d_sprite_free(self->sprite);

	free(self);


}

Entity* monsterEntityNew(GFC_Vector2D position)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a monster entity!");
	}

	self->sprite = gf2d_sprite_load_all("images/space_bug.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;
	self->think = monsterThink;
	self->update = monsterUpdate;
	self->free = monsterFree;
	self->touch = monsterTouch;
	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;
	self->free = monsterFree;

	//self->data = gfc_allocate_array(sizeOf(struct MonsterData), 1);
	//if (data)
	{

	}



	return self;

}

