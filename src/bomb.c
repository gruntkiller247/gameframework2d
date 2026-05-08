#include "entity.h"
#include <simple_json.h>
#include "simple_logger.h"
#include "bomb.h"
#include "projectiles.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

static const char* bombFile = "JSONs/bomb.json";

void bombThink(Entity* self);

void bombTouch(Entity* self, Entity* toucher);

void bombUpdate(Entity* self);

void bombFree(Entity* self);

void loadBomb(Entity* self);

/*
	-1 timeToLive = no die, 0 = 5 seconds
*/
Entity* bombEntityNew(GFC_Vector2D position, Uint8 team, int timeToLive)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		slog("Failed to spawn a bomb! for team %s", team);
		return NULL;
	}

	loadBomb(self);

	/*Stuff to data drive
	self->colorReal = GFC_COLOR_ORANGE;
	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->damage = 3;
	*/

	//Stuff to hardcode
	self->role = ROLE_BOMB;
	self->team = team;
	self->position = position;
	self->frame = 0;
	self->think = bombThink;
	self->free = bombFree;
	self->update = bombUpdate;
	self->touch = bombTouch;
	self->team = team;
	self->layer = EL_PROJECTILES;

	self->bounds = gfc_rect(0, 0, 32, 32);



	if (timeToLive == 0)
	{
		//slog("No time to live in BOMB!");
		self->timeToLive = 5;
		self->timerDeath = 0;
		/*data->timeToLive = 5;
		data->timerDeath = 0;*/
	}
	else if (timeToLive == -1)
	{
		//data->timerDeath = -1;
		self->timerDeath = -1;
	}
	else
	{
		self->timeToLive = timeToLive;
		self->timerDeath = 0;
		//data->timeToLive = timeToLive;
		//data-> timerDeath = 0;
	}

	self->move = 0;

	
	return self;
}

Entity* bombEntityNewSpecial(GFC_Vector2D position, Uint8 team, int* timeToLive)
{
	Entity* self;
	self = bombEntityNew(position, team, timeToLive);

	self->isSpecialBomb = 1;
}


void bombThink(Entity* self)
{
	if (!self)
		return;
	
	if (self->ultIs > 0 && self->ultIs <= 50)
	{
		//self->scale = gfc_vector2d(self->ultIs, self->ultIs);
		if(self->ultIs % 5 == 0)
			bakerExplode(self);

		//self->bounds = gfc_rect(0, 0, 32 * self->ultIs, 32 * self->ultIs);
		self->ultIs += 1;
		//slog("My damage is %i", self->damage);
	}
	else if (self->ultIs > 50)
	{
		//Kill itself
		self->ultIs = 0;
		self->_inUse = 0;
		//slog("Gunner ult killing self!");
	}
	else
		;

	if (self->timerDeath != -1)
	{
		self->timerDeath += 1;

		//slog("Timer Death: %i", self->timerDeath);


		if (self->timerDeath >= self->timeToLive)
		{

			explode(self);
			self->_inUse = false;
			//slog("I am the bomb and I am about to blow up!");
			return;
		}
	}

	if (self->move != 0)
	{
		if (self->velocity.y)
		{
			self->position.y += self->velocity.y;
			//slog("Boming moving on Y!");
		}

		if (self->velocity.x )
		{
			self->position.x += self->velocity.x;
			//slog("Bomb moving on X!");
		}

		if (self->velocity.y || self->velocity.x )
		{
			gfc_vector2d_normalize(&self->velocity);
		}
	}
	else
	{
		//slog("I AM A BOMB AND I SHOULD NOT BE MOVING!");
	}

}

void bombTouch(Entity* self, Entity* toucher) 
{
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
		//slog("Bomb is touching something! %s",toucher->name);
	}
}

void bombUpdate(Entity* self)
{

	if (!self)
		return;

	//addToCell(self);

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

void bombFree(Entity* self) 
{
	if (!self)
		return;

	//slog("Bomb is being killed!");

	if (self->sprite)
	{
		gf2d_sprite_free(self->sprite);
	}

	if (self->data)
		free(self->data);

	/*if (self->color)
		free(self->color);*/

	free(self);
}


void explode(Entity* self)
{
	//slog("I AM TRYING TO EXPLODE");
	Entity* N = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* NE = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* E = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* SE = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* S = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* SW = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* W = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);
	Entity* NW = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive, ROLE_PROJECTILE);


	if (!N || !NE || !E || !SE || !S || !SW || !W || !NW)
	{
		slog("Bomb Explode can't spawn bombs!");
		return;
	}

	//N, NE, E, SE, S ,SW, W ,NW

	if (self->damage / 3 > 0)
	{
		N->damage = self->damage / 3;
		NE->damage = self->damage / 3;
		E->damage = self->damage / 3;
		SE->damage = self->damage / 3;

		S->damage = self->damage / 3;
		SW->damage = self->damage / 3;
		W->damage = self->damage / 3;
		NW->damage = self->damage / 3;
	}
	else
	{
		N->damage = 1;
		NE->damage = 1;
		E->damage = 1;
		SE->damage = 1;

		S->damage = 1;
		SW->damage = 1;
		W->damage = 1;
		NW->damage = 1;
	}

	/*N->team = self->team;
	NE->team = self->team;
	E->team = self->team;
	SE->team = self->team;

	S->team = self->team;
	SW->team = self->team;
	W->team = self->team;
	NW->team = self->team;*/

	moveProjectile(N, D_NORTH);
	moveProjectile(NE, D_NORTHEAST);
	moveProjectile(E, D_EAST);
	moveProjectile(SE, D_SOUTHEAST);

	moveProjectile(S, D_SOUTH);
	moveProjectile(SW, D_SOUTHWEST);
	moveProjectile(W, D_WEST);
	moveProjectile(NW, D_NORTHWEST);
		

	if(self->ultIs == -1)
		self->_inUse = 0;

	//Might be a reason I did not write this initially - If I want 1 bomb to explode many times
	//self->_inUse;
}

void moveBomb(Entity* self, int direction)
{
	if (!self)
		return;

	//slog("Inside Projectile Move! Entity Name: %s",self->name);

	switch (direction)
	{
	case(D_NORTH):
		self->velocity.y -= self->topSpeed.y;
		break;

	case(D_NORTHEAST):
		self->velocity.y -= self->topSpeed.y;
		self->velocity.x += self->topSpeed.x;
		break;

	case(D_EAST):
		self->velocity.x += self->topSpeed.x;
		break;

	case(D_SOUTHEAST):
		self->velocity.y += self->topSpeed.y;
		self->velocity.x += self->topSpeed.x;
		break;

	case(D_SOUTH):
		self->velocity.y += self->topSpeed.y;
		break;

	case(D_SOUTHWEST):
		self->velocity.y += self->topSpeed.y;
		self->velocity.x -= self->topSpeed.x;
		break;

	case(D_WEST):
		self->velocity.x -= self->topSpeed.x;
		break;

	case(D_NORTHWEST):
		self->velocity.y -= self->topSpeed.y;
		self->velocity.x -= self->topSpeed.x;
		break;

	default:

		slog("No real direction given for projectile movement!");
		return;


	}
	return;
}


/*
	Only to be called by the Baker's Ultimate!
*/
void bakerExplode(Entity* self)
{
	Entity* N = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* NE = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* E = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* SE = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* S = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* SW = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* W = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* NW = bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);

	//N, NE, E, SE, S ,SW, W ,NW

	if (!N || !NE || !E || !SE || !S || !SW || !W || !NW)
	{
		slog("Baker Ult can't spawn bombs!");

		if (N)
		{
			N->_inUse = 0;
		}

		if (NE)
		{
			NE->_inUse = 0;
		}

		if (E)
		{
			E->_inUse = 0;
		}

		if (SE)
		{
			SE->_inUse = 0;
		}

		if (S)
		{
			S->_inUse = 0;
		}

		if (SW)
		{
			SW->_inUse = 0;
		}

		if (W)
		{
			W->_inUse = 0;
		}

		if (NW)
		{
			NW->_inUse = 0;
		}
		return;
	}
	
	N->damage = self->damage;
	NE->damage = self->damage;
	E->damage = self->damage;
	SE->damage = self->damage;

	S->damage = self->damage;
	SW->damage = self->damage;
	W->damage = self->damage;
	NW->damage = self->damage;
	
	N->move = 1;
	NE->move = 1;
	E->move = 1;
	SE->move = 1;

	S->move = 1;
	SW->move = 1;
	W->move = 1;
	NW->move = 1;

	/*N->team = self->team;
	NE->team = self->team;
	E->team = self->team;
	SE->team = self->team;

	S->team = self->team;
	SW->team = self->team;
	W->team = self->team;
	NW->team = self->team;*/
	

	moveBomb(N, D_NORTH);
	moveBomb(NE, D_NORTHEAST);
	moveBomb(E, D_EAST);
	moveBomb(SE, D_SOUTHEAST);

	

	moveBomb(S, D_SOUTH);
	moveBomb(SW, D_SOUTHWEST);
	moveBomb(W, D_WEST);
	moveBomb(NW, D_NORTHWEST);
}

void loadBomb(Entity* self)
{
	SJson* json = NULL;
	SJson* bjson = NULL;
	int damage = -1;
	const char* color = NULL;
	const char* spriteFile = NULL;


	if (!self)
		return;

	json = sj_load(bombFile);

	if (!json)
	{
		slog("Failed to load level's JSON!");
		return NULL;
	}

	bjson = sj_object_get_value(json, "bomb");

	if (!bjson)
	{
		slog("Failed to load the bomb's data from JSON!");
		goto fail;
	}

	spriteFile = sj_object_get_string(bjson, "sprite");

	if (!spriteFile)
	{
		slog("Bomb has no sprite!");
		goto fail;
	}


	if (sj_object_get_int(bjson, "damage", &damage) == 0)
	{
		slog("Error finding Bomb Damage");
		goto fail;
	}
	
	color = sj_object_get_string(bjson, "color");

	if (getColor(self, color) == 0)
	{
		//Means that the color was not found IE not a macro, for now whatever!
	}
	
	

	//sj_free(bjson);
	sj_free(json);

	return;

	fail:

	if (json)
		sj_free(json);
}