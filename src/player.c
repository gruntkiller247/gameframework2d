
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

typedef struct PD
{
	float timerPrimary;				//Timer that counts up to cooldown
	float primaryCooldown;			//Time until primary attack can be fired
	int basicPlayerProjectileLife;  //Projectile timer to live cap for the Player

	int timerDeath;				//Timer to count up to timeToLive
	int timeToLive;				//Time to Live for projectiles like things. Can be NULL;

}Player_Data; //Currently Cut content until I can fix this



Entity* playerEntityNew(GFC_Vector2D position)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a player!");
	}

	/*Player_Data* data = malloc(sizeof(Player_Data));

	if (!data)
	{
		//return NULL;
		//slog("Failed to allocate memory for player's data!");
	}*/


	//This will be all the baseline stats for the player. Class specific stuff will be in the class function call
	strcpy(self->name,"Matt");

	self->sprite = gf2d_sprite_load_all("images/ed210.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = playerThink;
	self->free = playerFree;
	self->update = playerUpdate;
	self->touch = playerTouch;

	self->velocity = gfc_vector2d(0,0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(30, 30, 72, 72);

	self->team = TEAM_PLAYER;

	self->timerPrimary = 0;
	self->primaryCooldown = 50;

	self->basicPlayerProjectileLife = 1000;
	self->hp = 3;

	self->hitDelay = 300;
	self->hitTimer = 0;
	self->isInvul = 0;

	
	/*
	data->timerPrimary = 0;
	data->primaryCooldown = 50;
	
	data->basicPlayerProjectileLife = 1000;

	self->data = data;
	*/

	return self;

}



void playerThink(Entity* self)
{
	
	if (!self)
		return;

	self->timerPrimary += 1.0;
	//slog("TimerPrimary is %i,", self->timerPrimary);
	//slog("primaryCooldown is %i,", self->primaryCooldown);

	if (gfc_input_key_down("UP") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		_playerShoot(self,D_UP);
	}

	if (gfc_input_key_down("DOWN") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		_playerShoot(self,D_DOWN);
	}

	if (gfc_input_key_down("LEFT") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		_playerShoot(self,D_LEFT);
	}
	
	if (gfc_input_key_down("RIGHT") && self->timerPrimary >= self->primaryCooldown)
	{
		self->timerPrimary = 0;
		//slog("Should be shooting a thing!");
		_playerShoot(self,D_RIGHT);
	}



	if (gfc_input_key_down("d"))
	{
		self->position.x += 1;
		//self->rotation = 180;
		//self->basicPlayerProjectileLife = 180;

	}

	if (gfc_input_key_down("a"))
	{
		self->position.x -= 1;
		//self->rotation = 0;
		//self->lastShotRotation = 0;

	}

	if (gfc_input_key_down("s"))
	{
		self->position.y += 1;
		//self->rotation = 270;
		//self->lastShotRotation = 270;

	}

	if (gfc_input_key_down("w"))
	{
		self->position.y -= 1;
		//self->rotation = 90;
		//self->lastShotRotation = 90;

	}

	if (self->velocity.y || self->velocity.x)
	{		
		gfc_vector2d_normalize(&self->velocity);
		//gfc_vector2d_scale(self->velocity, self->velocity, self->topSpeed);
	}


	//Player Damage Checking
	if (self->isInvul == 1 && self->hitTimer <= self->hitDelay)
	{
		self->hitTimer += 1;
		//slog("Monster is immune, has been for %i", self->hitTimer);
	}
	else
	{
		self->isInvul = 0;
		self->hitTimer = 0;
		//slog("Monster is no longer immune!");
	}

	//slog("Last shot rotation: %i", self->lastShotRotation);

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
		if (toucher->team = TEAM_ENEMY && self->isInvul == 0) 
		{
			self->hp -= 1;
			self->isInvul = 1;
			slog("Enemy aligned thing touched me %s. HP is now %i", self->name, self->hp);

		}
	}

}

void playerUpdate(Entity* self)
{
	if (!self)
		return;

	//slog("Updateing Player!");

	self->frame += 0.1;

	if (self->frame >= 8)
		self->frame = 0;

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
void _playerShoot(Entity* self,int direction)
{
	if (!self)
		return;

	Entity* thing = projectileEntityNew( gfc_vector2d(self->position.x+self->bounds.x,self->position.y+self->bounds.y), TEAM_PLAYER,self->basicPlayerProjectileLife);

	if (!thing)
	{
		slog("Failed to spawn a projectile when firing player!");
		return;
	}

	strcpy(thing->name, "Player's Pew!");

	//slog("INSIDER! Last shot rotation: %i", self->lastShotRotation);
	
	if (direction == D_LEFT)
	{
		thing->velocity.x -= 10;
		//180: right - d
		//slog("Fireing Left!");
	}
	else if(direction == D_RIGHT)
	{
		thing->velocity.x += 10;
		//0: left - a
		//slog("Fireing Right!");
	}
	else if(direction == D_DOWN)
	{
		thing->velocity.y += 10;
		//270 down - s
		//slog("Fireing down!");
	}
	else if (direction == D_UP)
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

void playerGunnerShoot(Entity* self, int direction)
{

}

void playerGunnterSpecial(Entity* self) 
{

}

void playerGunnerUltimate(Entity* self)
{

}