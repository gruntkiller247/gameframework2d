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

typedef struct
{
	Entity* player;
	MonsterStates state;
}MonsterData;

void monsterThink(Entity* self);

void monsterFree(Entity* self)
{
	MonsterData* data;

	if (!self)
		return;

	data = self->data;
	//clean up anything I own

	free(data);


}

Entity* monsterEntityNew(GFC_Vector2D position)
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
	self->think = monsterThink;
	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;
	self->free = monsterFree;

	//self->data = gfc_allocate_array(sizeOf(MonsterData), 1);
	//if (data)
	{

	}


	//self->think = playerThink();
	//self->free = playerFree();
	//self->update = playerUpdate();

	return self;

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