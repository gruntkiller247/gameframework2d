#include "boss.h"
#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"



Entity* bossEntityNew(GFC_Vector2D position, int role)
{
	Entity* self;
	self = entityNew();

	self->team = TEAM_ENEMY;
	self->layer = EL_MONSTER;

	switch (role)
	{
		case ROLE_BOSS1:

			break;

		case ROLE_BOSS2:

			break;

		case ROLE_BOSS3:

			break;

		default:

			slog("Error! Boss spawning without a class!");
			return NULL;
	}
}





