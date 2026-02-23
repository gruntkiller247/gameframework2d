#ifndef __ROLE_H__
#define __ROLE_H__

#include "simple_logger.h"
#include "entity.h"
//#include "entity.h"
//This class forms the basis for player's class
// and monster/boss classes

typedef enum RN
{
	ROLE_PLAYER_GUNNER,
	ROLE_PLAYER_MAGE,
	ROLE_PLAYER_WARRIOR,
	ROLE_TRASHMOB,

}RoleNames;



void roleSelect(Entity* self,int className);


#endif