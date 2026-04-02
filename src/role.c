#include "simple_logger.h"
#include "entity.h"
#include "role.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>

//Gonna depericate this stuff for now
//Load in the stats for the class
void roleSelect(Entity* self, int className)
{	
	/*
	int min = 1;
	int max = 3;

	srand(time(NULL));
	int r = rand() % (max - min + 1) + min;*/

	if (!self)
	{
		slog("Can't give a class to nobody!");
	}
	switch (className)
	{
	case ROLE_PLAYER_GUNNER:

		//self->fire = player.playerGunnerShoot;
		break;

	//case ROLE_PLAYER_MAGE:

		//break;

	//case ROLE_PLAYER_WARRIOR:

		//break;

	default:

		slog("Forgot to make the spot for that class!");

	}

	return;
}