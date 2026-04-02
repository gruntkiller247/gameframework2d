
#include <stdio.h>
#include <time.h>

#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_shape.h"
#include "gfc_input.h"|
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "projectiles.h"
#include "bomb.h"
#include <stdlib.h>

typedef struct PD
{
	void (*fire)(struct Entity_S* fire, int direction);			//Describes how the entity attacks
	void (*special)(struct Entity_S* special, int direction);	//Describes how the entity uses their special attack
	void (*ultimate)(struct Entity_S* ultimate);				//Describes how the entity uses their ultimate

	int ultLength;												//Lenght of time the gambler's ULT last for


}PlayerData; //Currently Cut content until I can fix this


static int baseSpeedMod = 3;

Entity* playerEntityNew(GFC_Vector2D position, int role)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a player!");
	}

	PlayerData* data = malloc(sizeof(PlayerData));

	if (!data)
	{
		slog("Failed to allocate memory for player's data!");
		return NULL;
	}


	//This will be all the baseline stats for the player. Class specific stuff will be in the class function call
	strcpy(self->name,"The Player");
	self->role = role;

	self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = playerThink;
	self->free = playerFree;
	self->update = playerUpdate;
	self->touch = playerTouch;

	self->currentPowerUp = ROLE_PU_NONE;

	self->velocity = gfc_vector2d(1,1);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(30, 30, 72, 72);

	self->team = TEAM_PLAYER;
	self->layer = EL_PLAYER;


	self->basicPlayerProjectileLife = 1000;
	self->maxHP = 3;
	self->hp = self->maxHP;

	self->hitDelay = 300;
	self->hitTimer = 0;
	self->isInvul = 0;

	self->damage = 1;

	//Player Roll Stuff
	//int role = roleSelect(self,ROLE_PLAYER_GUNNER);


	//Gunner numbers
	self->timerPrimary = 0;
	self->primaryCooldown = 50;

	self->timerSpecial = 0;
	self->specialCooldown = 100;
	self->specialDamage = 3;

	self->timerUlt = 0;
	self->ultCooldown = 100;
	self->ultDamage = 5;
	self->ultIs = 0;
	
	switch (role)
	{
		case ROLE_PLAYER_GUNNER:
			self->role = role;
			data->fire = playerGunnerShoot;
			data->special = playerGunnerSpecial;
			data->ultimate = playerGunnerUltimate;
			break;

		case ROLE_PLAYER_BAKER:
			self->role = role;
			self->bombAmount = 20;
			self->bombTLL = 100;
			data->fire = playerBakerShoot;
			data->special = playerBakerSpecial;
			data->ultimate = playerBakerUlt;
		
			break;

		case ROLE_PLAYER_GAMBLER:
			self->role = role;
			self->ultIs = 0;
			data->fire = playerGamblerShoot;
			data->special = playerGamblerSpecial;
			data->ultimate = playerGamblerUlt;
			data->ultLength = 10;


			break;

		default:
		slog("Player has no role!");

	}

	self->data = data;
	setPlayer(self);

	return self;

}



void playerThink(Entity* self)
{
	
	if (!self)
		return;

	PlayerData* data = (PlayerData*)self->data;

	if (!data)
	{
		slog("Player lost their data! Killing them!");
		self->_inUse = 0;
		return;
	}

	//slog("Player Position: X = %f Y= %f", self->position.x, self->position.y);
	self->timerPrimary += 1;
	self->timerSpecial += 1;
	self->timerUlt += 1;
	//slog("TimerPrimary is %i,", self->timerPrimary);
	//slog("primaryCooldown is %i,", self->primaryCooldown);

	//slog("Counter for Ult: %i", self->timerUlt);
	if (gfc_input_key_pressed("v") && self->timerUlt >= self->ultCooldown)
	{
		//slog("Trying to Ult!");
		data->ultimate(self);
		self->timerUlt = 0;
	}


	if (gfc_input_key_held("z") && self->timerSpecial >= self->specialCooldown)
	{
		//slog("Trying to fire gunner special!");
		self->timerSpecial = 0;


		if (gfc_input_key_held("UP"))
		{
			data->special(self, D_NORTH);
		}
		else if (gfc_input_key_held("DOWN"))
		{
			data->special(self, D_SOUTH);
		}	
		else if (gfc_input_key_held("LEFT"))
		{
			data->special(self, D_WEST);
		}
		else
		{
			data->special(self, D_EAST);
		}
	}

	if (gfc_input_key_pressed("UP") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		data->fire(self,D_NORTH);
	}

	if (gfc_input_key_pressed("DOWN") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		data->fire(self,D_SOUTH);
	}

	if (gfc_input_key_pressed("LEFT") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		data->fire(self,D_WEST);
	}
	
	if (gfc_input_key_pressed("RIGHT") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		data->fire(self,D_EAST);
	}



	if (gfc_input_key_down("d"))
	{
		self->position.x += self->velocity.x;
		//self->rotation = 180;
		//self->basicPlayerProjectileLife = 180;

	}

	if (gfc_input_key_down("a"))
	{
		self->position.x -= self->velocity.x;
		//self->rotation = 0;
		//self->lastShotRotation = 0;

	}

	if (gfc_input_key_down("s"))
	{
		self->position.y += self->velocity.y;
		//self->rotation = 270;
		//self->lastShotRotation = 270;

	}

	if (gfc_input_key_down("w"))
	{
		self->position.y -= self->velocity.y;
		//self->rotation = 90;
		//self->lastShotRotation = 90;

	}


	

	//Player Damage Checking
	if (self->isInvul == 1 && self->hitTimer <= self->hitDelay)
	{
		self->hitTimer += 1;
		//slog("Player is immune, has been for %i", self->hitTimer);
	}
	else
	{
		self->isInvul = 0;
		self->hitTimer = 0;
		//slog("Monster is no longer immune!");
	}

	//player powerup checker
	if (self->currentPowerUp != ROLE_PU_NONE)
	{
		if (self->powerUpTimer < self->powerUpMaxTime)
			self->powerUpTimer += 1;
		else
		{
			//Kill the powered up state!
			slog("Disabling the power up!");
			//slog("Current role for powerup is: %i", self->currentPowerUp);

			switch (self->currentPowerUp)
			{
			case ROLE_PU_FREE_ULT:

				break;

			case ROLE_PU_INVUL:

				break;

			case ROLE_PU_HP_RECOVERY:

				break;

			case ROLE_PU_SPEED:
				self->velocity = gfc_vector2d(1, 1);
				break;

			case ROLE_PU_BOMB:

				break;

			default:
				slog("Player picked up a bad powerup!");
				return;

			}


			self->powerUpTimer = 0;
			self->currentPowerUp = ROLE_PU_NONE;
		}
	}
	else if (self->velocity.y || self->velocity.x)
	{
		gfc_vector2d_normalize(&self->velocity);
		self->velocity.x *= baseSpeedMod;
		self->velocity.y *= baseSpeedMod;
		//gfc_vector2d_scale(self->velocity, self->velocity, self->topSpeed);
	}
	else
		;

	//slog("Last shot rotation: %i", self->lastShotRotation);
	//slog("Player Position: X = %f Y= %f", self->position.x, self->position.y);

}

void playerTouch(Entity* self, Entity* toucher)
{
	//Player Collision
	if (!self || !toucher)
		return;

	float selfLeft = self->position.x + self->bounds.x;
	float selfRight = selfLeft + self->bounds.w;
	float selfTop = self->position.y + self->bounds.y;
	float selfBottom = selfTop + self->bounds.h;

	float toucherLeft = toucher->position.x + toucher->bounds.x;
	float toucherRight = toucherLeft + toucher->bounds.w;
	float toucherTop = toucher->position.y + toucher->bounds.y;
	float toucherBottom = toucherTop + toucher->bounds.h;

	if (selfLeft < toucherRight && selfRight > toucherLeft && selfTop  < toucherBottom && selfBottom > toucherTop)
	{
		//slog("%s is touching something!",self->name);
		
		
		if (toucher->team == TEAM_ENEMY && toucher->layer !=EL_SYMBOLS &&  self->isInvul == 0) 
		{
			self->hp -= 1;
			self->isInvul = 1;
			slog("Player being touched %s. \nHP is now %i \nToucher team is: %i\n Toucher's role is %i", toucher->name, self->hp,toucher->team,toucher->role);

		}

		if (toucher->team == TEAM_ITEM && self->currentPowerUp == ROLE_PU_NONE)
		{
			self->currentPowerUp = toucher->currentPowerUp;
			toucher->_inUse = 0;
			playerPowerUps(self, toucher);

		}
	}

}

void playerUpdate(Entity* self)
{
	if (!self)
		return;

	//slog("Updateing Player!");
	PlayerData* data = (PlayerData*)self->data;

	if (!data)
	{
		slog("Player lost their data! Killing them!");
		self->_inUse = 0;
		return;
	}

	self->frame += 0.1;

	if (self->frame >= 8)
		self->frame = 0;

	//gambler Ult stuff
	if (self->ultIs == 1)
	{
		slog("Gambler Ult is on!");

		if (self->timerUlt >= data->ultLength)
		{
			self->ultIs == 0;
			
		}
	}

	//Does not work for intended purpose, but makes a funny leash from world origin to player
	//gf2d_draw_line(gfc_vector2d(self->position.x, self->position.y), gfc_vector2d(self->bounds.x, self->bounds.y), GFC_COLOR_RED);

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

void playerFree(Entity* self)
{
	if (!self)
		return;

	slog("Player is being freed!");

	if (self->sprite)
		gf2d_sprite_free(self->sprite);

	if (self->data)
		free(self->data);

	free(self);
}

/*
	To be called in think during state fire. 
	Test method deperciated
*/
void playerShoot(Entity* self,int direction)
{
	if (!self)
		return;

	Entity* thing = projectileEntityNew( gfc_vector2d(self->position.x+self->bounds.x,self->position.y+self->bounds.y), TEAM_PLAYER,self->basicPlayerProjectileLife, ROLE_PROJECTILE);

	if (!thing)
	{
		slog("Failed to spawn a projectile when firing player!");
		return;
	}

	strcpy(thing->name, "Pew!");

	//slog("INSIDER! Last shot rotation: %i", self->lastShotRotation);
	
	if (direction == D_WEST)
	{
		thing->velocity.x -= 10;
		//180: right - d
		//slog("Fireing Left!");
	}
	else if(direction == D_EAST)
	{
		thing->velocity.x += 10;
		//0: left - a
		//slog("Fireing Right!");
	}
	else if(direction == D_SOUTH)
	{
		thing->velocity.y += 10;
		//270 down - s
		//slog("Fireing down!");
	}
	else if (direction == D_NORTH)
	{
		thing->velocity.y -= 10;
		//90 up
		//slog("Fireing Up!");
	}
	else
	{
		slog("Fireing Nowhere!");
	}

	return;

}

/*
	Spawns a bomb at the player's feet on the player's team!
*/
void makeBomb(Entity* self)
{
	//slog("Spawning a bomb for testing!");

	//slog("Player Position: X = %f Y= %f", self->position.x, self->position.y);
	
	float rX = (self->position.x) + (float)rand() / RAND_MAX * (self->bounds.w*2);
	float rY = (self->position.y) + (float)rand() / RAND_MAX * (self->bounds.h*2);


	//slog("Random spot: X = %i Y = %i",rX,rY);

	Entity* thing = bombEntityNewSpecial(self->position, TEAM_PLAYER, 300);

	//Entity* thing2 = bombEntityNewSpecial(gfc_vector2d(rX, rY), TEAM_PLAYER, 300);
	//thing->scale = gfc_vector2d(5,5);
	//explode(thing);

	return;
}

void playerGunnerShoot(Entity* self, int direction)
{
	if (!self)
		return;

	Entity* thing = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
	Entity* thing2 = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
	Entity* thing3 = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
	//thing->damage = 2;


	if (!thing)
	{
		slog("Failed to spawn a projectile when firing Gunner!");
		return;
	}

	//strcpy(thing->name, strcat(self->name,"'s Pew!"));
	//strcpy(thing2->name, strcat(self->name, "'s Pews!"));
	//strcpy(thing3->name, strcat(self->name, "'s Pews!"));

	switch (direction)
	{
		case(D_WEST):
			thing->velocity.x -= 10;


			thing2->velocity.x -= 10;
			thing2->velocity.y -= 5;

			thing3->velocity.x -= 10;
			thing3->velocity.y += 5;
			break;

		case(D_EAST):
			thing->velocity.x+=10;

			thing2->velocity.x += 10;
			thing2->velocity.y += 5;

			thing3->velocity.x += 10;
			thing3->velocity.y -= 5;
			break;

		case(D_SOUTH):
			thing->velocity.y += 10;

			thing2->velocity.y += 10;
			thing2->velocity.x -= 5;

			thing3->velocity.y += 10;
			thing3->velocity.x += 5;
			break;

		case(D_NORTH):
			thing->velocity.y -= 10;

			thing2->velocity.y -= 10;
			thing2->velocity.x -= 5;

			thing3->velocity.y -= 10;
			thing3->velocity.x += 5;
			break;

		default:
			slog("Something went wrong during Gunner Shoot!");
	}
		
	

	return;
}

void playerGunnerSpecial(Entity* self,int direction) 
{
	if (!self)
		return;
	//slog("Firing gunner special!");

	Entity* thing = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
	thing->scale = gfc_vector2d(5,5);
	thing->bounds= gfc_rect(0, 0, 32*5, 32*5);
	thing->damage = self->specialDamage;

	switch (direction)
	{
	case(D_WEST):
		thing->velocity.x -= 10;
		
		break;

	case(D_EAST):
		thing->velocity.x += 10;
		

		break;

	case(D_SOUTH):
		thing->velocity.y += 10;

		
		break;

	case(D_NORTH):
		thing->velocity.y -= 10;

		
		break;

	default:
		slog("Something went wrong during Gunner SPECIAL!");
	}
}

void playerGunnerUltimate(Entity* self)
{
	//slog("Firing Gunner Ult!");
	Entity* thing = projectileEntityNew(gfc_vector2d(self->position.x * -1 + self->bounds.x * -2, self->position.y * -1 + self->bounds.y * -2), TEAM_PLAYER, -1, ROLE_PROJECTILE);
	thing->scale = gfc_vector2d(0, 0);
	thing->bounds = gfc_rect(0, 0, 32*0, 32*0);
	thing->damage = self->ultDamage;
	thing->ultIs = 1;

	//return thing;
}

void playerBakerShoot(Entity* self, int direction)
{
	slog("Baker Shooting");
	if (!self)
		return;

	//Same as gunner, shoots bombs that have a short range/life
	Entity* thing = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->bombTLL);

	slog("Baker shooting! Bomb team is %i", thing->team);

	//thing->scale = gfc_vector2d(5, 5);
	//thing->bounds = gfc_rect(0, 0, 32 * 5, 32 * 5);
	thing->damage = self->damage;
	thing->move = 1;

	switch (direction)
	{
	case(D_WEST):
		thing->velocity.x -= 10;

		break;

	case(D_EAST):
		thing->velocity.x += 10;


		break;

	case(D_SOUTH):
		thing->velocity.y += 10;


		break;

	case(D_NORTH):
		thing->velocity.y -= 10;


		break;

	default:
		slog("Something went wrong during Baker Shoot!");
	}

}

//This special does not use a direction
void playerBakerSpecial(Entity* self, int direction)
{
	slog("Baker Special");
	
	float rX, rY;
	Entity* thing;
	int c;

	for (c = 0; c <= self->bombAmount; c++)
	{
		rX = (self->position.x) + (float)rand() / RAND_MAX * (self->bounds.w * 2);
		rY = (self->position.y) + (float)rand() / RAND_MAX * (self->bounds.h * 2);

		thing = bombEntityNew(gfc_vector2d((int)rX, (int)rY), TEAM_PLAYER, self->bombTLL*2);
		//thing->move = -1;

		if (!thing)
		{
			slog("Failed to make a bomb in playerBakerSpecial");
			return;
		}

		//slog("Created a bomb! It's velocitys are (%f,%f)", thing->velocity.x, thing->velocity.y);
	}

}

void playerBakerUlt(Entity* self)
{
	slog("Baker Ult");
	
	Entity* thing = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->bombTLL*2);
	thing->scale = gfc_vector2d(0, 0);
	thing->bounds = gfc_rect(0, 0, 32 * 0, 32 * 0);
	thing->damage = self->ultDamage;
	thing->ultIs = 1;
	//thing->move = 1;

}

void playerGamblerShoot(Entity* self, Uint8 direction)
{
	if (!self)
		return;

	Entity* thing = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), TEAM_PLAYER, self->basicPlayerProjectileLife, ROLE_PROJECTILE);

	//slog("Gambler Shooting");

	if (self->ultIs == 1)
	{
		
		thing->damage = 3;
		//return;
	}

	int num = rand() % 10;

	if (num <= 7)
	{
		
		thing->damage = 1;
	}
	else if (num <= 9)
	{
		
		thing->damage = 2;
	}
	else
	{
		
		thing->damage = 3;
	}

	//thing->damage = 3;
	slog("Damage is %i", thing->damage);

	switch (direction)
	{
	case(D_WEST):
		thing->velocity.x -= 10;

		break;

	case(D_EAST):
		thing->velocity.x += 10;


		break;

	case(D_SOUTH):
		thing->velocity.y += 10;


		break;

	case(D_NORTH):
		thing->velocity.y -= 10;


		break;

	default:
		slog("Something went wrong during Gabmler Shoot!");
	}

	return;

	
}

void playerGamblerSpecial(Entity* self, int direction)
{
	if (!self)
		return;

	slog("Gambler Special... Nothing!");
}

void playerGamblerUlt(Entity* self, int direction)
{
	if (!self)
		return;

	self->ultIs = 1;

	slog("Gambler Ult");
}

/*
	self is the player, role is the role of the powerup!
*/
void playerPowerUps(Entity* self, Entity* powerup)
{
	if (!self || !powerup)
		return;

	int role = powerup->role;

	self->powerUpMaxTime = powerup->powerUpMaxTime;

	switch (role)
	{
		case ROLE_PU_FREE_ULT:
			if (rand() % 4 == 1)
			{
				self->timerUlt = self->ultCooldown;
				slog("Player is lucky, giving them a free ult!");
			}
			else
				slog("Player was unlucky! No ult for you!");

			self->currentPowerUp = ROLE_PU_NONE;

			break;

		case ROLE_PU_INVUL:
			self->isInvul = 1;
			self->currentPowerUp = ROLE_PU_NONE;

			slog("Player picked up invul powerup!");
			break;

		case ROLE_PU_HP_RECOVERY:
			self->hp += 1;
			self->currentPowerUp = ROLE_PU_NONE;
			break;

		case ROLE_PU_SPEED:
			self->velocity = gfc_vector2d(5,5);

			//self->topSpeed = gfc_vector2d(5, 5);
			self->currentPowerUp = ROLE_PU_SPEED;
			slog("Player picked up speed powerup!");

			break;

		case ROLE_PU_BOMB:

			makeBomb(self);
			self->currentPowerUp = ROLE_PU_NONE;
			break;

		default:
			slog("Player picked up a bad powerup!");
			self->currentPowerUp = ROLE_PU_NONE;
			return;

	}
	//self->currentPowerUp = ROLE_PU_NONE;
}