#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"

typedef struct Entity_S
{
	Uint8 _inUse;								//No Touch
	GFC_TextLine name;							//Name of the entity
	GFC_Vector2D position;						//Coordinates in 2d space
	GFC_Vector2D scale;
	float rotation;
	Sprite* sprite;
	float frame;
	GFC_Vector2D topSpeed;
	GFC_Vector2D velocity;
	void	(*think)(struct Entity_S *self);	//Called every frame if defined for the entity
	void	(*update)(struct Entity_S* self);	
	void	(*free)(struct Entity_S* self);		
	void	(*data) (struct data);
}Entity;

/*
	@brief initialize the entity sub system
	@param max the upper limit for concurrent entities
*/
void entityManagerInit(Uint32 max);


/*
	@brief get a pointer to a new free entity
	@return Null if out of entities, a pointer to a blank entity otherwise
*/
Entity* entityNew();


/*
	@brief free a previously allocated entity
	@para self the entity to free
	@note do not use the memory address again after calling it
*/
void entityFree(Entity* self);

void entityDraw(Entity* self);

void entityUpdateAll();

void entityThinkAll();

void entityManagerDrawAll();

#endif
