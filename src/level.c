#include "simple_logger.h"
#include <simple_json.h>
#include "level.h"
#include "gfc_list.h"
#include "entity.h"
#include "monster.h"
#include "player.h"
#include "projectiles.h"
#include "bomb.h"


static int viewWidth = 1200;
static int viewHeight = 720;

Level* levelNew(Uint64 height, Uint64 width)
{
	Level* level;
	level = gfc_allocate_array(sizeof(Level), 1);

	if (!level)
	{
		slog("Failed to create a level!");
		return NULL;
	}

	if (!width || !height)
	{
		slog("Cannot make a level with bounds of 0");
		return NULL;
	}

	//Put code here later to make the map be boundless!
	//This requires making a real camera system!

	if (height > 720 || width > 1200)
	{
		slog("Clamped Height max! Can't make level!");
		return NULL;
	}

	level->height = height;
	level->width = width;
	level->levelMap = gfc_list_new();
	level->background = NULL;
}

void levelFree(Level* level)
{
	if (!level)
		return;

	if (level->background)
	{
		gf2d_sprite_free(level->background);
	}

	free(level->levelMap);
	free(level);
}

void levelDraw(Level* level)
{
	if (!level)
		return;

	if (!level->background)
	{
		slog("Level has no background image!");
		return;
	}

	gf2d_sprite_draw_image(level, gfc_vector2d(0, 0));
}

void levelSetBackground(Level* level,Sprite* background)
{
	level->background = background;
}

void levelSetHeight(int maxHeight, int maxWidth)
{
	viewHeight = maxHeight;
	viewWidth = maxWidth;
}

/*
	Helper function to parse the JSON to get the correct enum role
*/
int getRole(const char* role)
{
	if (!role)
	{
		slog("Role has a bad pointer!");
		return ROLE_ERROR;
	}

	if (strcmp(role, "ROLE_TRASHMOB") == 0)
		return ROLE_TRASHMOB;
	else if (strcmp(role, "ROLE_PROJECTILE") == 0)
		return ROLE_PROJECTILE;
	else if (strcmp(role, "ROLE_BOMB") == 0)
		return ROLE_BOMB;
	else if (strcmp(role, "ROLE_BOSS1") == 0)
		return ROLE_BOSS1;
	else if (strcmp(role, "ROLE_BOSS2") == 0)
		return ROLE_BOSS2;
	else if (strcmp(role, "ROLE_BOSS3") == 0)
		return ROLE_BOSS3;
	else if (strcmp(role, "ROLE_PLAYER_GAMBLER") == 0)
		return ROLE_PLAYER_GAMBLER;
	else if (strcmp(role, "ROLE_PLAYER_BAKER") == 0)
		return ROLE_PLAYER_BAKER;
	else if (strcmp(role, "ROLE_PLAYER_GUNNER") == 0)
		return ROLE_PLAYER_GUNNER;
	else
		return ROLE_ERROR;
	
}

Level* dataLoadLevel(const char* levelName)
{
	Level* level = NULL;
	SJson* json = NULL;
	SJson* ljson = NULL;
	SJson* entity = NULL;
	SJson* entities = NULL;
	GFC_Vector2D* position;
	Entity* temp = NULL;
	const char* name = NULL;
	const char* background= NULL;
	int team =0, time=0;
	int c=0,role=0,entityMax=0;
	int tempX =0 , tempY =0;


	if (!levelName)
	{
		slog("No level name supplied!");
		return NULL;
	}

	level = levelNew(viewHeight, viewWidth);

	json = sj_load(levelName);

	if (!json)
	{
		slog("Failed to load level's JSON!");
		return NULL;
	}

	ljson = sj_object_get_value(json, "level");

	if (!ljson)
	{
		slog("%s could not load level data: missing level data!", levelName);
		sj_free(json);
		return NULL;
	}

	
	background=sj_object_get_string(ljson, "background");

	if (!background)
	{
		slog("Background failed to load!");
		goto fail;
	}
	
	level->background = _strdup(background);
	if (!level->background) {
		slog("Failed to allocate memory for background string!");
		goto fail;
	}


	entities = sj_object_get_value(ljson, "entities");

	if (!entities)
	{
		slog("Entities list could not be made!");
		sj_free(ljson);
		return NULL;
	}

	if (gfc_list_get_count(entities) == 0)
	{
		slog("Entities list is 0?");
		sj_free(ljson);
		sj_free(entities);
		return NULL;
			
	}
	
	entityMax = sj_array_get_count(entities);

	for (c = 0; c < entityMax; c++)
	{
		//Switch through each roll to make the entity/monster/player that should be made per item
		//Manually give that entity their name and position
		//slog("Loop %i!", c);
		entity = sj_array_get_nth(entities, c);

		role = getRole(sj_object_get_string(entity, "role"));

		if (sj_object_get_int(entity, "positionY", tempY) == 0)
		{
			slog("Error finding position Y");
			goto fail;
		}

		if (sj_object_get_int(entity, "positionX", tempX) == 0)
		{
			slog("Error finding postion X");
			goto fail;
		}
		
		slog("Position of thing from json X:%i Y:%i", tempX, tempY);
		

		position = gfc_vector2d_new(tempX, tempY);

		if (!position)
		{
			slog("Position is NULL!");
			goto fail;
		}

		name = sj_object_get_string(entity,"name");

		switch (role)
		{
			case ROLE_TRASHMOB:
				temp=monsterEntityNew(*position,role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
					


				break;
			case ROLE_PROJECTILE:
				 sj_object_get_int(entity, "time", time);

				if (!time)
				{
					slog("Failed to load projectile's time to die!");
					goto fail;
				}

				 sj_object_get_int(entity, "team", team);

				if (!team)
				{
					slog("Failed to load projectile's team!");
					goto fail;
				}

				temp = projectileEntityNew(*position, team,time,role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_BOMB:
				sj_object_get_int(entity, "time", time);

				if (!time)
				{
					slog("Failed to load bomb's time to die!");
					goto fail;
				}

				sj_object_get_int(entity, "team", team);

				if (!team)
				{
					slog("Failed to load projectile's team!");
					goto fail;
				}

				temp=bombEntityNew(*position,team,time);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);

				break;

			case ROLE_BOSS1:
				temp = monsterEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_BOSS2:
				temp = monsterEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_BOSS3:
				temp = monsterEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_PLAYER_GAMBLER:
				temp = playerEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				
				break;

			case ROLE_PLAYER_GUNNER:

				temp = playerEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_PLAYER_BAKER:

				temp = playerEntityNew(*position, role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			default:
				slog("Role parsed error, no entity created from JSON file!");
		}
	}

	slog("Read the entire JSON!");
	//sj_free(background);
	//sj_free(entities);
	//sj_free(ljson);
	sj_free(json);
	
	

	return level;

fail:
	//slog("Hit a fail condition!");


	if (entities)
		sj_free(entities);

	if (ljson)
		sj_free(ljson);



	if (json)
		sj_free(json);

	return NULL;

}