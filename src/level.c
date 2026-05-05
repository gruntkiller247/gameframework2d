#include <simple_logger.h>
#include <SDL_ttf.h>
#include <simple_json.h>
#include "level.h"
#include <gfc_list.h>
#include "entity.h"
#include "monster.h"
#include "player.h"
#include "projectiles.h"
#include "bomb.h"
#include "powerup.h"
#include "button.h"
#include "gf2d_graphics.h"
#include "gfc_list.h"




static int viewWidth = 1200;
static int viewHeight = 720;

const char* errorBackground = "images/backgrounds/bg_flat.png";
static SDL_Color uiColor = { 255, 255, 255, 255 };
#define MY_FONT "fonts/FreeSans.ttf"

static TTF_Font* font;
static int levelStatus = LS_NORMAL;

static Level* currentLevel = NULL;
static const char* nextLevel = "";


void initalizeLevel()
{
	font = TTF_OpenFont(MY_FONT, 64);
	if (!font)
		slog("FONT DID NOT LOAD IN LEVEL!");
}

//extern struct EntityManager* entityManager;
/*
typedef struct
{
	
	Level* levelList;
	Uint32 levelMax;
	Uint32 currentLevel;
}LevelManager;


static LevelManager levelManager = { 0 };


void levelManagerInit(Uint32 max)
{
	if (!max)
	{
		slog("You cannot initalize Level system with 0 entities");
		return;
	}
	
	levelManager.levelList = gfc_allocate_array(sizeof(Level), max);

	if (!levelManager.levelList)
	{
		slog("Failed to allocate Level array!");
		return;
	}
	levelManager.levelMax = max;


	font = TTF_OpenFont(MY_FONT, 64);
	if (!font)
		slog("FONT DID NOT LOAD IN LEVEL!");

	atexit(levelManagerClose);
	slog("Initalized Level System");
}

void levelManagerClose()
{
	Level* level;
	int c;

	if (!levelManager.levelMax)
		return NULL;
	
	for (c = 0; c < levelManager.levelMax; c++)
	{
		levelFree(&levelManager.levelList[c]);
	}

	memset(&levelManager, 0, sizeof(levelManager));
	slog("Closed UI System");
}*/


/*
	Makes a level of height and width. Does not read any json files!
*/
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

	/*level->levelUI = gfc_list_new();

	if (!level->levelUI)
	{
		slog("Level failed to allocate UI List!");
		return NULL;
	}
	
	/*level->levelMap = gfc_list_new();

	if (!level->levelMap)
	{
		slog("Failed to allocate Level Map for Level!");
		return NULL;
	}*/


	level->height = height;
	level->width = width;

	level->background = NULL;
	level->spawnPowerUps = 1;

	return level;
}

void levelFree(Level* level)
{
	if (!level)
		return;
	//slog("Insider Level Free");
	void* thing = NULL;
	int c = 0;

	if (level->background)
	{
		//slog("Freeing background");
		gf2d_sprite_free(level->background);
		level->background = NULL;
		//slog("Post Free background!");
	}

	/*if (level->levelMap)
	{
		slog("Free levelmap");
		for (int i = 0; i < gfc_list_get_count(level->levelMap); i++)
		{
			thing = gfc_list_get_nth(level->levelMap, i);
		}
		gfc_list_delete(level->levelMap);
		gfc_list_delete(level->levelMap);
	}*/

	/*if (level->levelUI)
	{
		slog("Freeing levelUI!");
		for ( c = 0; c < gfc_list_get_count(level->levelUI); c++)
		{
			thing = gfc_list_get_nth(level->levelUI, c);
		}
		gfc_list_delete(level->levelUI);
	}*/
	
	free(level);
}

void levelKillAll()
{

	levelFree(currentLevel);
	/*int c;

	if (!levelManager.levelList)
	{
		return;
	}

	for (c = 0; c < levelManager.levelMax; c++)
	{
		levelFree(&levelManager.levelList[c]);
	}*/
}

void levelDraw(Level* level)
{
	if (!level)
	{
		slog("Level was NULL! Cannot draw a NULL level!");
		return;
	}
		

	if (!level->background)
	{
		slog("Level has no background image!");
		return;
	}

	gf2d_sprite_draw_image(level->background, gfc_vector2d(0, 0));
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
	else if (strcmp(text, "UI_TEXT") == 0)
	{
		return UI_TEXT;
	}
	else if (strcmp(text, "UI_IMAGE") == 0)
	{
		return UI_IMAGE;
	}
	else
	{
		return UI_ERROR;
	}

}

/*
	Creates a level and reads in values from the JSON file provided!
*/
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

	const char* uiSprite = NULL;
	const char* uiHoverSprite = NULL;
	const char* uiClickSprite = NULL;
	const char* onClick = NULL;
	const char* uiText = NULL;

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
	int activeOnPause = -1;
	UI* tempUI = NULL;

	SDL_Rect uiRect;
	SDL_Surface* uiSurface = NULL;
	SDL_Texture* tempTexture = NULL;
	
	const char* spawn = NULL;
	int spawnRole = -1;


	

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
			slog("Error getting UI array count or it is 0!");
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

				onClick = sj_object_get_string(uiElement, "onClick");

				if (!onClick)
				{
					slog("Failed to get onClick for UI!");
					tempUI->onClick = NULL;
				}

				
				tempUI = newButton(gfc_vector2d(uiPosX, uiPosY), gfc_rect(uiBX, uiBY, uiBW, uiBH),uiType,onClick);

				if (!tempUI)
				{
					slog("Failed to create UI object!");
					goto fail;
				}


	

				//"spawn" : "ROLE_TRASHMOB",
				spawn = sj_object_get_string(uiElement, "spawn");

				if (!spawn)
				{
					slog("Button has nothing to spawn!");
				}
				else
				{
					slog("Role to spawn: %s",spawn);
					spawnRole = getRole(spawn);
					if (spawnRole != ROLE_ERROR)
					{
						tempUI->spawn = spawnRole;
					}
					else
					{
						slog("Failed to find spawnRole for spawn!");
						tempUI->spawn = ROLE_ERROR;
					}
				}

				uiSprite = sj_object_get_string(uiElement, "sprite");

				if (!uiSprite)
				{
					slog("uiSprite failed to load!");
					uiText = sj_object_get_string(uiElement, "text");

					if (!uiText)
					{
						slog("Button has neither sprite nor text!");
						goto fail;
					}
					else
					{
						//Button with text instead of a sprite!
						tempUI->text = uiText;
						uiSurface = TTF_RenderText_Solid(font, uiText, uiColor);

						if (!uiSurface)
						{
							slog("Failed to create Surface for UI!");
							slog(SDL_GetError());
							goto fail;
						}

						if (!gf2d_graphics_get_renderer())
						{
							slog("Level does not have graphics renderer!");
							goto fail;
						}

						tempTexture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), uiSurface);


						if (!tempTexture)
						{
							slog("No UI texture created!");
							return;
						}


						updateTexture(tempUI, tempTexture);


						SDL_FreeSurface(uiSurface);
					}
				}
				else
				{
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
				}

				



				if (sj_object_get_int(uiElement, "activeOnPause", &activeOnPause) == 0)
				{
					slog("Failed to find activeOnPause for UI! Defaulting!");
					tempUI->activeOnPause = 0;
					//goto fail;
				}

				tempUI->activeOnPause = activeOnPause;


				
				//gfc_list_append(level->levelUI, tempUI);



				break;
			case UI_TEXT:

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

				uiRect.x = uiBX;
				uiRect.y = uiBY;
				uiRect.h = uiBH;
				uiRect.w = uiBW;

				tempUI = newButton(gfc_vector2d(uiPosX, uiPosY), gfc_rect(uiBX, uiBY, uiBW, uiBH), uiType, onClick);

				uiText = sj_object_get_string(uiElement, "text");
				
				if (!uiText)
				{
					slog("UI Element has no text!");
					tempUI->text = "NULL";
				}
				else
					tempUI->text = uiText;
	

				uiSurface = TTF_RenderText_Solid(font, uiText, uiColor);

				if (!uiSurface)
				{
					slog("Failed to create Surface for UI!");
					slog(SDL_GetError());
					goto fail;
				}

				if (!gf2d_graphics_get_renderer())
				{
					slog("Level does not have graphics renderer!");
					goto fail;
				}

				tempTexture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), uiSurface);


				if (!tempTexture)
				{
					slog("No UI texture created!");
					return;
				}


				updateTexture(tempUI, tempTexture);
				
			
				SDL_FreeSurface(uiSurface);

				//tempUI->active = 1;
				//gfc_list_append(level->levelUI, tempUI);
				break;

			default:
				slog("Error reading UI! Not a real UI type!");
			}
		}

		
	}

	
	background=sj_object_get_string(ljson, "background");

	//slog("\n\nLevel Loading: Background is: %s\n\n", background);

	if (!background)
	{
		slog("Background failed to load!");
		background = _strdup(errorBackground);
		//goto fail;
	}
	
	level->background = gf2d_sprite_load_image(background);//_strdup(background);

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
		//sj_free(ljson);
		//return NULL;
		goto levelLoaded;	
	}

	if (gfc_list_get_count(entities) == 0)
	{
		slog("Entities list is 0?");
		//sj_free(ljson);
		//sj_free(entities);
		//return NULL;
		goto levelLoaded;
			
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

		if(sj_object_get_int(entity, "delay", &delay) == 0)
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

	levelLoaded:
	
	currentLevel = level;
	//gfc_list_prepend(levelManager.levelList, level);
	//levelManager.currentLevel = gfc_list_get_item_index(levelManager.levelList, level);
	//slog("Level Manger current level: %i", levelManager.currentLevel);

	slog("Loaded Level JSON!");


	
	sj_free(json);

	//slog("Post JSON FREE!");
	//sj_free(background);
	//sj_free(entities);
	//sj_free(ljson);
	
	
	//This Crashes, but needs to be done at a some point!
	//SDL_FreeSurface(uiSurface);

	//if (uiTexture)
		//SDL_FreeSurface(uiTexture);
	

	
	return level;

fail:
	slog("Hit a fail condition when loading the Level JSON!");
	//slog("WE ARE IN THE FAIL CONDITION LMAO!");


	if (temp)
		temp->_inUse = 0;

	if (uiSurface)
		SDL_FreeSurface(uiSurface);

	/*if (entiFties)
		sj_free(entities);

	if (ljson)
		sj_free(ljson);*/

	if (json)
		sj_free(json);

	if (level)
		levelFree(level);

	return NULL;

}

void saveLevel()
{
	SJson* json = NULL;
	SJson* ljson = NULL;
	SJson* entitiesArray = NULL;
	SJson* uiArray = NULL;
	SJson* temp = NULL;

	int c = 0;
	int posX = 0;
	int posY = 0;
	int delay = 0;
	int time = 0;
	int count = 0;

	const char* role = NULL;
	const char* name = NULL;
	const char* team = NULL;

	Entity* dude = NULL;
	GFC_List* entityData = NULL;

	json = sj_object_new();

	if (!json)
	{
		slog("Failed to allocate new JSON file!");
		return;
	}

	ljson = sj_object_new();

	if (!ljson)
	{
		slog("Failed to make Level part of JSON!");
		return;
	}

	entitiesArray = sj_array_new();

	if (!entitiesArray)
	{
		slog("Failed to make entity JSON array!");

		goto fail;
	}

	uiArray = sj_array_new();

	if (!uiArray)
	{
		slog("Failed to make UI JSON array!");

		if (entitiesArray)
			sj_free(entitiesArray);
		goto fail;
	}

	



	sj_object_insert(ljson, "background", sj_new_str(errorBackground));
	sj_object_insert(ljson, "name", sj_new_str("New Custom Level!"));


	entityData = gfc_list_new();

	if (!entityData)
	{
		slog("Failed to make gfc list");
		goto fail;
	}

	getEntityData(entityData);

	if (!entityData)
	{
		slog("Failed to get entity data from entity manager!");
		goto fail;
	}

	//JSON obj -> json array -> json obj

	
	count = gfc_list_get_count(entityData);
	for (c = 0; c<count; c++)
	{
		//slog("C is: %i", c);
		dude = gfc_list_get_nth(entityData, c);

		if (!dude)
		{
			slog("Failed to create Entity Data from array!");

			if (entitiesArray)
				sj_free(entitiesArray);

			if (uiArray)
				sj_free(uiArray);


			goto fail;
		}
			
		
		temp = sj_object_new();

		if (!temp)
		{
			if (entitiesArray)
				sj_free(entitiesArray);

			if (uiArray)
				sj_free(uiArray);

			goto fail;
		}

	
		

		posX = dude->position.x;
		//slog("Got PosX!");
		sj_object_insert(temp,"positionX", sj_new_int(posX));

		posY = dude->position.y;
		//slog("Got PosY!");
		sj_object_insert(temp, "positionY", sj_new_int(posY));

		if (!dude->name)
		{
			name = "NO NAME";
		}
		else
			name = dude->name;

		sj_object_insert(temp, "name", sj_new_str(name));
		
		//slog("Got Name!");
		
		role = getRoleFromInt(dude->role);
		sj_object_insert(temp, "role", sj_new_str(role));
		//slog("Got Role!");
		
		delay = dude->delay;
		//slog("Got Delay!");
		sj_object_insert(temp, "delay", sj_new_int(delay));

		

		switch (dude->role)
		{
			case ROLE_TRASHMOB:
				break;

			case ROLE_BOSS1:
				break;

			case ROLE_BOSS2:
				break;

			case ROLE_BOSS3:
				break;

			case ROLE_PROJECTILE:

			case ROLE_BOMB:
				
				team = getTeamFromInt(dude->team);
				sj_object_insert(temp, "team", sj_new_str(team));
				
				time = dude->timeToLive;
				sj_object_insert(temp, "time", sj_new_int(time));

				break;

			case ROLE_PU_BOMB:

			case ROLE_PU_FREE_ULT:

			case ROLE_PU_HP_RECOVERY:

			case ROLE_PU_INVUL:

			case ROLE_PU_RANDOM:
				team = getTeamFromInt(dude->team);
				sj_object_insert(temp, "team", sj_new_str(team));
				break;

			case ROLE_PLAYER_BAKER:
				break;

			case ROLE_PLAYER_GAMBLER:
				break;

			case ROLE_PLAYER_GUNNER:
				break;

			default:
				
				slog("Failed to find role when Saving custom Level!");
				
				break;
		}
		sj_array_append(entitiesArray,temp);
		
		
	}

	sj_object_insert(ljson, "entities", entitiesArray);
	sj_object_insert(json, "level", ljson);
	slog("Trying to save JSON!");
	
	sj_save(json,"levels/New_Custom_Level.level");

	slog("Post save attempt!");
	
	sj_free(json);

	return;
	
	fail:
	slog("Failed to save the level!");


	if (json)
		sj_free(json);

	if (uiArray)
		sj_free(uiArray);

	if (temp)
		sj_free(temp);

	return;
}

void levelKillLevel()
{
	
}

void levelUpdate(const char* levelName)
{
	nextLevel = levelName;
	levelStatus = LS_NEW_LEVEL;
}

int getLevelStatus()
{
	return levelStatus;
}

void setLevelStatus(int status)
{
	levelStatus = status;
}

Level* getCurrentLevel()
{
	return currentLevel;//&levelManager.levelList[levelManager.currentLevel];
}

void setNextLevel(const char* name)
{
	if (!name)
	{
		nextLevel = NULL;
		return;
	}

	
	strcpy(nextLevel, name);
}

const char* getNextLevel()
{
	return nextLevel;
}