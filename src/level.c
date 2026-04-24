#include "simple_logger.h"
#include <simple_json.h>
#include "level.h"
#include "gfc_list.h"
#include "entity.h"
#include "monster.h"
#include "player.h"
#include "projectiles.h"
#include "bomb.h"
#include "powerup.h"
#include "button.h"


static int viewWidth = 1200;
static int viewHeight = 720;

Level* levelNew(Uint64 height, Uint64 width)
{

	//Put code here later to make the map be boundless!
	//This requires making a real camera system!

	if (height > 720 || width > 1200)
	{
		slog("Can't make level! Height or Width is too large!");
		return NULL;
	}

	if (!width || !height)
	{
		slog("Cannot make a level with bounds of 0");
		return NULL;
	}

	Level* level;
	level = malloc(sizeof(Level));

	if (!level)
	{
		slog("Failed to create a level!");
		return NULL;
	}


	level->levelUI = gfc_list_new();

	if (!level->levelUI)
	{
		slog("Level failed to allocate UI List!");
		return NULL;
	}

	level->levelMap = gfc_list_new();

	if (!level->levelMap)
	{
		slog("Failed to allocate Level Map for Level!");
		return NULL;
	}


	level->height = height;
	level->width = width;

	level->background = NULL;
	level->spawnPowerUps = 1;
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

int getTeam(const char* team)
{
	if (!team)
	{
		slog("Team has a bad pointer!");
		return TEAM_NONE;
	}

	if (strcmp(team, "TEAM_PLAYER") == 0)
		return TEAM_PLAYER;
	else if (strcmp(team, "TEAM_ENEMY") == 0)
		return TEAM_ENEMY;
	else if (strcmp(team, "TEAM_ITEM") == 0)
		return TEAM_ITEM;
	else
		return TEAM_NONE;
		
}

int getUIType(const char* text)
{
	if (!text)
	{
		slog("NULL Text from JSON UI Level!");
		return UI_ERROR;
	}
	
	if (strcmp(text, "UI_BUTTON") == 0)
	{
		return UI_BUTTON;
	}
	else
	{
		return UI_ERROR;
	}

}



Level* dataLoadLevel(const char* levelName)
{
	Level* level = NULL;
	SJson* json = NULL;
	SJson* ljson = NULL;
	SJson* entity = NULL;
	SJson* entities = NULL;
	SJson* uiElement = NULL;

	SJson* ui = NULL;

	GFC_Vector2D* position;
	Entity* temp = NULL;
	const char* name = NULL;
	const char* background= NULL;
	const char* team = NULL;
	const char* levelObjName = NULL;
	const char* uiTypeString = NULL;
	const char* onClick = NULL;

	const char* uiSprite = NULL;
	const char* uiHoverSprite = NULL;
	const char* uiClickSprite = NULL;

	int time= 0;
	int c=0, role=0, entityMax=0;
	int tempX =0 , tempY =0;
	int tempInt = -1;
	int teamEnum = -1;
	int delay = -1;

	int uiType = NULL;
	int uiPosX = -1;
	int uiPosY = -1;
	int uiBX = -1;
	int uiBY = -1;
	int uiBW = -1;
	int uiBH = -1;
	UI* tempUI = NULL;



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

	//UI Reading!
	ui = sj_object_get_value(ljson, "UI");

	if (!ui)
	{
		slog("Either no UI or UI is NULL!");
	}
	else
	{
		slog("UI Exist!");
		entityMax = sj_array_get_count(ui);

		if (!entityMax)
		{
			slog("Error getting UI array count!");
			goto fail;
		}

		for (c = 0; c < entityMax; c++)
		{
			uiElement = sj_array_get_nth(ui, c);
			uiType = getUIType(sj_object_get_string(uiElement, "type"));

			if (uiType == UI_ERROR)
			{
				slog("Error parsing UI Type!");
				goto fail;
			}

			onClick = sj_object_get_string(uiElement, "onClick");

			if (!onClick)
			{
				slog("onClick either NULL or Not Found!");
			}


			switch (uiType)
			{
			case UI_BUTTON:


				if (sj_object_get_int(uiElement, "positionX", &uiPosX) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}

				if (sj_object_get_int(uiElement, "positionY", &uiPosY) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}


				if (sj_object_get_int(uiElement, "boundsX", &uiBX) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}

				if (sj_object_get_int(uiElement, "boundsY", &uiBY) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}

				if (sj_object_get_int(uiElement, "boundsW", &uiBW) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}

				if (sj_object_get_int(uiElement, "boundsH", &uiBH) == 0)
				{
					slog("Failed to create Level UI!");
					goto fail;
				}

				tempUI = newButton(gfc_vector2d(uiPosX, uiPosY), gfc_rect(uiBX, uiBY, uiBW, uiBH),uiType,onClick);

				if (!tempUI)
				{
					slog("Failed to create UI object!");
					goto fail;
				}

				uiSprite = sj_object_get_string(uiElement, "sprite");

				if (!uiSprite)
				{
					slog("uiSprite failed to load!");
					goto fail;
				}

				tempUI->sprite = gf2d_sprite_load_all(uiSprite, 128, 128, 16, 0);



				uiClickSprite = sj_object_get_string(uiElement, "onClickSprite");

				if (!uiClickSprite)
				{
					//slog("onClickSprite failed to load!");
					tempUI->onClickSprite = NULL;
					//goto fail;
				}
				else
					tempUI->onClickSprite = gf2d_sprite_load_all(uiClickSprite, 128, 128, 16, 0);



				uiHoverSprite = sj_object_get_string(uiElement, "onHoverSprite");

				if (!uiHoverSprite)
				{
					//slog("uiHoverSprite failed to load!");
					tempUI->onHoverSprite = NULL;
					//goto fail;
				}
				else
					tempUI->onHoverSprite = gf2d_sprite_load_all(uiHoverSprite, 128, 128, 16, 0);



				gfc_list_append(level->levelUI, tempUI);


				break;

			default:
				slog("Error reading UI! Not a real UI type!");
			}
		}

		
	}

	
	background=sj_object_get_string(ljson, "background");

	if (!background)
	{
		slog("Background failed to load!");
		goto fail;
	}
	
	level->background = _strdup(background);
	if (!level->background) 
	{
		slog("Failed to allocate memory for background string!");
		goto fail;
	}

	levelObjName = sj_object_get_string(ljson, "name");

	if (!levelObjName)
	{
		slog("Level JSON file has no name! Giving default temp name!");
		level->name = "Default Level Name!";
	}
	else
	{
		//slog("Found the Level's Name!");
		level->name = _strdup(levelObjName);
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
		//slog("JSON: Entity role: %i", role);

		if (sj_object_get_int(entity, "positionY", &tempY) == 0)
		{
			slog("Error finding position Y");
			goto fail;
		}

		if (sj_object_get_int(entity, "positionX", &tempX) == 0)
		{
			slog("Error finding postion X");
			goto fail;
		}
		
		//slog("Position of thing from json X:%i Y:%i", tempX, tempY);
		

		position = gfc_vector2d_new(tempX, tempY);

		if (!position)
		{
			slog("Position is NULL!");
			goto fail;
		}

		name = sj_object_get_string(entity,"name");

		if(sj_object_get_int(entity, "delay", &delay) == 0);
		{
			//slog("Either error getting a delay value, or delay is 0!");
		}

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
				
				teamEnum=getTeam(sj_object_get_string(entity, "team"));
				

				sj_object_get_int(entity, "time", &time);

				temp = projectileEntityNew(*position, teamEnum,time,role);

				if (!temp)
				{
					slog("Failed to create entity in loading level switch statement!");
					goto fail;
				}

				if (name)
					strcpy(temp->name, name);
				break;

			case ROLE_BOMB:
				teamEnum = getTeam(sj_object_get_string(entity, "team"));

				//slog("Bomb Team is: %i", teamEnum);

				sj_object_get_int(entity, "time", &time);

				//slog("Bomb Time is: %i", time);

				temp=bombEntityNew(*position, teamEnum,time);

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

			case ROLE_PU_RANDOM:

				temp = powerUpEntityNew(*position, role);

				break;
				
			case ROLE_PU_HP_RECOVERY:

				temp = powerUpEntityNew(*position, role);

				break;

			case ROLE_PU_INVUL:

				temp = powerUpEntityNew(*position, role);

				break;

			case ROLE_PU_BOMB:

				temp = powerUpEntityNew(*position, role);

				break;
			
			case ROLE_PU_SPEED:

				temp = powerUpEntityNew(*position, role);

				break;
			
			case ROLE_PU_FREE_ULT:

				temp = powerUpEntityNew(*position, role);

				break;
					

			default:
				slog("Role parsed error, no entity created from JSON file!");
		}

		if (temp)
		{
			temp->position = gfc_vector2d(tempX,tempY);
			
			temp->delay = delay;
			temp->delayTimer = 0;
		}
	}

	slog("Loaded Level JSON!");

	//sj_free(background);
	//sj_free(entities);
	//sj_free(ljson);
	sj_free(json);
	
	

	return level;

fail:
	//slog("Hit a fail condition!");

	if (level)
		levelFree(level);

	if (temp)
		temp->_inUse = 0;

	/*if (entities)
		sj_free(entities);

	if (ljson)
		sj_free(ljson);*/

	if (json)
		sj_free(json);

	return NULL;

}

Level* dataLoadMainMenu(const char* levelPath)
{
	if (!levelPath)
	{
		slog("No filepath to main menu!");
		return NULL;
	}


	fail:
		return NULL;
}