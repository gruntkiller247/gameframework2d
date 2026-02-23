#include "simple_logger.h"
#include "entity.h"
#include "role.h"

void roleSelect(Entity* self, int className)
{
	//3 loadouts for the player
	// Each one has 1 primary attack, 1 charged special, 1 charging ultimate
	//Mage
	//Warrior
	//Gunner - shoots in compass directions, shoots a barage in a direction, deploys turrent that shoots at something?

	if (!self)
	{
		slog("Can't give a class to nobody!");
	}

	switch (className)
	{
		case ROLE_PLAYER_GUNNER:

		break;
		
		case ROLE_PLAYER_MAGE:

		break;

		case ROLE_PLAYER_WARRIOR:
			
		break;

		default:

		slog("Forgot to make the spot for that class!");

	}

	return;
}