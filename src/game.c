#include <SDL.h>
#include "simple_logger.h"
#include <stdio.h>
#include <time.h>
#include <SDL_ttf.h>
#include "gfc_input.h"
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "player.h"
#include "monster.h" 
#include "world.h"
#include "gf2d_draw.h"
#include "projectiles.h"
#include "bomb.h"
#include "powerup.h"
#include "level.h"

#define MY_FONT "fonts/FreeSans.ttf"

//Static bullshit
//Window Size
static int viewWidth = 1200;
static int viewHeight = 720;
static int renderWdith = 1200;
static int renderHeight = 720;

//Drawing for UI
static char playerHP[100];
static char bossHP[100];
static SDL_Surface* surface;
static SDL_Texture* texture;
static SDL_Surface* surfaceBoss;
static SDL_Texture* textureBoss; 
static SDL_Rect dstRect;    //For the Player's Health UI
static SDL_Rect dstRect2;   //For the Boss's Health UI

static char fpsNum[100];
static SDL_Surface* fpsSurface;
static SDL_Texture* fpsTexture;
static SDL_Rect fpsRect;

Entity* bossGame = NULL;
Entity* player = NULL;



void fillEntityManager()
{
    int c;
    Entity* thing;

    for (c = 0; c < 5000; c++)
    {

            thing = bombEntityNew(gfc_vector2d(100, 100),TEAM_PLAYER,0);
        

    }
}

void loadLevel(Uint8 level,Uint8 playerRole)
{
    entityKillAll();

    player = playerEntityNew(gfc_vector2d(0, 0), playerRole);
    strcpy(player->name, "Player");

    switch (level)
    {
    case -1:
        slog("Testing data driven level!");
        dataLoadLevel("level1.level");
    case 0:
        slog("Loadding main menu!");

    case  1:
        slog("Loading Testing Level");
        /*Entity* projectile;
           projectile = projectileEntityNew(gfc_vector2d(300, 0), TEAM_ENEMY, -1);
           projectile->team = TEAM_IGNORE;
           strcpy(projectile->name, "TEST_PROJECTILE");*/

        Entity* enemy;
        enemy = monsterEntityNew(gfc_vector2d(300, 100), ROLE_TRASHMOB);
        strcpy(enemy->name, "Mr Monster!");
        enemy->hp = 10;
        setMonsterState(enemy, -1);

        //Entity* bomb;
        //bomb = bombEntityNew(gfc_vector2d(500,0),TEAM_PLAYER,-1);
        //bomb->team = TEAM_IGNORE;
        //strcpy(bomb->name, "TEST_BOMB!");

        Entity* powerup;
        powerup = powerUpEntityNew(gfc_vector2d(200, 500), PU_FREE_ULT);
        strcpy(powerup->name, "Free Ult Power Up");

        Entity* powerup2;
        powerup2 = powerUpEntityNew(gfc_vector2d(100, 500), PU_SPEED);
        strcpy(powerup2->name, "Speed Power Up");

        Entity* powerup3;
        powerup3 = powerUpEntityNew(gfc_vector2d(300, 500), PU_BOMB);
        strcpy(powerup3->name, "Bomb Power Up");

        Entity* powerup4;
        powerup4 = powerUpEntityNew(gfc_vector2d(400, 500), PU_HP_RECOVERY);
        strcpy(powerup4->name, "HP Power Up");

        Entity* powerup5;
        powerup5 = powerUpEntityNew(gfc_vector2d(500, 500), PU_INVUL);
        strcpy(powerup5->name, "Invul Power Up");
        break;

    case 2:
        slog("Loading Boss 1");
        //powerUpSpawning = 1;

        bossGame = monsterEntityNew(gfc_vector2d(300, 100), ROLE_BOSS1);
        strcpy(bossGame->name, "POS");
        bossGame->hp = 10;

        break;

    case 3:
        slog("Loading Boss 2");
        //powerUpSpawning = 1;

        bossGame = monsterEntityNew(gfc_vector2d(300, 100), ROLE_BOSS2);
        strcpy(bossGame->name, "POS2");
        bossGame->hp = 10;

        break;

    case 4:
        slog("Loading Boss 3");
        //powerUpSpawning = 1;

        bossGame = monsterEntityNew(gfc_vector2d(300, 100), ROLE_BOSS3);
        strcpy(bossGame->name, "POS3");
        bossGame->hp = 10;

        break;

    default:
        slog("No Level Loaded!");
    }
}

void framerateUI(TTF_Font* font,SDL_Color color,SDL_Rect** frameBox)
{
    //gf2d_graphics_get_frames_per_second()

    if (!font || !frameBox)
    {
        slog("FPS UI Not working!");
        return;
    }

    snprintf(fpsNum, sizeof(fpsNum), "FPS: %i", (int)gf2d_graphics_get_frames_per_second());
    fpsSurface = TTF_RenderText_Solid(font, fpsNum, color);

    if (!fpsSurface)
    {
        slog("No fps surface to draw!");
        return;
    }

    fpsTexture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), fpsSurface);


    if (!fpsTexture)
    {
        slog("No fps texture to draw!");
        return;
    }

    fpsRect.x = 0;
    fpsRect.y = 680;
    fpsRect.w = fpsSurface->w / 2;
    fpsRect.h = fpsSurface->h / 2;
    SDL_FreeSurface(fpsSurface);


}

void updateUI( Uint8 playerNumHP, Uint8 bossNumHP, TTF_Font* font, SDL_Color color, SDL_Rect** playerHPUI, SDL_Rect** bossHPUI)
{
    if (!font || !playerHPUI)
    {
        slog("Font or player UI not working! Can't draw!");
        return;
    }

    if (!playerHPUI)
    {
        return;
    }

    if (!player)
    {
        slog("UI Has no Player!");
        return;
    }

    if (!bossGame || !bossHPUI)
    {
        //slog("UI Has no boss!");
    }

    //slog("Player hp is %i", player);

    snprintf(playerHP, sizeof(playerHP), "Player HP: %d", player->hp);
    surface = TTF_RenderText_Solid(font, playerHP, color);

    if (!surface)
    {
        slog("No player surface to draw!");
        return;
    }

    texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);

    if (!texture)
    {
        slog("No player texture to draw!");
        return;
    }

    dstRect.x = 10;
    dstRect.y = 10;
    dstRect.w = surface->w / 2;
    dstRect.h = surface->h / 2;
    SDL_FreeSurface(surface);

    if (!bossGame)
    {
        //slog("This level has no boss!");
    }
    else
    {
        snprintf(bossHP, sizeof(bossHP), "Boss HP: %d", bossGame->hp);

        surfaceBoss = TTF_RenderText_Solid(font, bossHP, color);

        if (!surfaceBoss)
        {
            slog("No boss surface to draw!");
            return;
        }

        textureBoss = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceBoss);

        if (!textureBoss)
        {
            slog("No boss texture to draw!");
            return;
        }

        dstRect2.x = 1000;
        dstRect2.y = 10;
        dstRect2.w = surfaceBoss->w / 2;
        dstRect2.h = surfaceBoss->h / 2;
        SDL_FreeSurface(surfaceBoss);
    }
        

    /*snprintf(playerHP, sizeof(playerHP), "Player HP: %i", playerNumHP);
    snprintf(bossHP, sizeof(bossHP), "Boss HP: %i", bossNumHP);

    surface = TTF_RenderText_Solid(font, playerHP, color);
    texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
    dstRect.x = 10;
    dstRect.y = 10;
    dstRect.w = surface->w / 2;
    dstRect.h = surface->h / 2;
    SDL_FreeSurface(surface);*/

    if (!bossHPUI || !bossNumHP)
    {
        //slog("No Boss for UI to draw!");
        return;
    }
    else
    {
        surfaceBoss = TTF_RenderText_Solid(font, bossHP, color);
        textureBoss = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceBoss);
        dstRect2.x = 1000;
        dstRect2.y = 10;
        dstRect2.w = surfaceBoss->w / 2;
        dstRect2.h = surfaceBoss->h / 2;
        SDL_FreeSurface(surfaceBoss);
    }

}

//128 x 128 grid for GIMP + snap to grid
int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    Sprite *sprite;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color(1, 1, 1, 100);//= gfc_color8(255,100,255,200);

    srand(time(NULL));
    TTF_Font* font = NULL;
    SDL_Color color = { 255, 255, 255, 255 };
    Uint8 playerRole = ROLE_PLAYER_GUNNER;

    int paused = 0;
    
    Level* currentLevel;

    //surfaceBoss = TTF_RenderText_Solid(font, bossHP, color);
    //textureBoss = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceBoss);
    

    int level = -1;
    int numPowerUps = 0;
    int powerUpSpawning = 0;
    int powerUpCounter = 0;
    int powerUpTime = 500;
    Entity* powerUpGame = NULL;

    int distance;

    //SDL_GetTikcs returns milli seconds program has been running


    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        viewWidth,
        viewHeight,
        renderWdith,
        renderHeight,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);
    TTF_Init();

    if (TTF_Init() == -1) {
        slog("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        slog("Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }
    font = TTF_OpenFont(MY_FONT, 64);
    if (!font)
        slog("FONT DID NOT LOAD!");
    
    
    entityManagerInit(16384);//2^14
    gfc_input_init("config/input.gfc");
    SDL_ShowCursor(SDL_DISABLE);
    

    /*demo setup*/
    //sprite = gf2d_sprite_load_image("images/backgrounds/cat.jpg");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");



    
    
    /*loadLevel(level, playerRole);
    player = playerEntityNew(gfc_vector2d(0, 0), ROLE_PLAYER_GAMBLER);
    strcpy(player->name, "Player");*/

    currentLevel = dataLoadLevel("levels/debugLevelProjectiles.level");
    

    if (!currentLevel)
    {
        slog("No level could be loaded! Loading error handling level!");
        sprite= gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
        loadLevel(1,ROLE_PLAYER_GAMBLER);
    }
    else
    {
        sprite = gf2d_sprite_load_image(currentLevel->background);
        if (currentLevel->spawnPowerUps)
            powerUpSpawning = 1;
    }


   
    player=getPlayer();
   
    
    /*main game loop*/
    while(!done)
    {
       
        gfc_input_update();
        //SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);

        if (!paused && level != 0)
        {
            if (gfc_input_key_pressed("g"))
            {
                slog("Pausing?");
                paused = 1;
            }
                

            mf += 0.1;
            if (mf >= 16.0)mf = 0;


            if (powerUpSpawning == 1/* && numPowerUps < 3*/)
            {
                if (rand() % 10000 == 1)
                {
                    slog("Spawning a power up!");
                    powerUpGame = powerUpEntityNew(gfc_vector2d(rand() % 1200, rand() % 720), -1);
                    //numPowerUps++;
                }

            }
            


            //update Thinking Here
            entityThinkAll();
            //slog("Thunk");


            gf2d_graphics_clear_screen();// clears drawing buffers
            // all drawing should happen betweem clear_screen and next_frame
                //backgrounds drawn first
            gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));


            entityManagerDrawAll();
            //slog("Drawn");


            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx, my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);


            entityTouchAll();
            //slog("Touched");  

            entityUpdateAll();
            //slog("Updated");

            entityBoundsCheckAll();
            //slog("Bounds checked!");

            entityFreeAll();
            //slog("Freed");
            
           
            if (!bossGame)
            {
                
                if (!player)
                    slog("NO PLAYER!");
                else
                    updateUI(player->hp, NULL, font, color, &dstRect, &dstRect2);
            }
            else
            {
                if (!player)
                    slog("NO PLAYER!");
                else
                    updateUI(player->hp, bossGame->hp, font, color, &dstRect, &dstRect2);
            }
                
            
            framerateUI(font,color,&fpsRect);
            //slog("UI updated");

            SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);
            

            //Render text
            SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &dstRect);
            SDL_RenderCopy(gf2d_graphics_get_renderer(), textureBoss, NULL, &dstRect2);
            SDL_RenderCopy(gf2d_graphics_get_renderer(), fpsTexture, NULL, &fpsRect);
            SDL_RenderPresent(gf2d_graphics_get_renderer());
            

            gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
            //slog("1 cycle!");
        }
        else if(level !=0)
        {
            if (gfc_input_key_pressed("g"))
            {
                slog("Unpausing?");
                paused = 0;
            } 

            if (gfc_input_key_pressed("1"))
            {
                //Clear this level then load 1
                level = 1;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("2"))
            {
                //Clear this level then load 2
                level = 2;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("3"))
            {
                //Clear this level then load 3
                level = 3;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("4"))
            {
                //Clear this level then load 4
                level = 4;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("p"))
            {
                slog("Changing Player's Class to Gambler!");
                playerRole = ROLE_PLAYER_GAMBLER;
                loadLevel(level, playerRole);
                
            }

            if (gfc_input_key_pressed("o"))
            {
                slog("Changing Player's Class to Baker!");
                playerRole = ROLE_PLAYER_BAKER;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("i"))
            {
                slog("Changing Player's Class to Gunner!");
                playerRole = ROLE_PLAYER_GUNNER;
                loadLevel(level, playerRole);
            }

            if (gfc_input_key_pressed("m"))
            {
                fillEntityManager();
            }

            gf2d_graphics_clear_screen();
            gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));
            entityManagerDrawAll();
           

            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx, my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

            if (!bossGame)
            {
                updateUI(player->hp, NULL, font, color, &dstRect, &dstRect2);
            }
            else
            {
                updateUI(player->hp, bossGame->hp, font, color, &dstRect, &dstRect2);
                distance = getDistance(player, bossGame);

                //slog("Boss and Player are %i units away!", distance);
            }

            framerateUI(font, color, &fpsRect);
             
            SDL_RenderCopy(gf2d_graphics_get_renderer(), fpsTexture, NULL, &fpsRect);
            SDL_RenderPresent(gf2d_graphics_get_renderer());
            
            gf2d_graphics_next_frame();
              
        }
        else
        {
            //Main menu
            
            gf2d_graphics_clear_screen();
            mf += 0.1;
            if (mf >= 16.0)mf = 0;

            gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx, my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);
            gf2d_graphics_next_frame();

            //Audio done through sdl2 mixer
            //Oggs for music
            //wavs for effects
            //background is mixed music

            //Must be initalized
            // Mix_PlayMusic -1 == loop forever 0 == 1
            //Mix_loadMus
        }
       
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
 

    entityKillAll();

    /*if (player)
    {
        slog("Freeing player post game");
        entityFree(player);
    }
        
    
    if (projectile)
    {
        slog("Freeing TEST PROJECTILE post game");
        entityFree(projectile);
    }
        
    
    if (enemy)
    {
        slog("Freeing TEST MONSTER post game");
        entityFree(enemy);
    }*/

   
    
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
    TTF_Quit();
    entityManagerClose();
    //slog("entityManager is closed!");
    //monsterManagerClose();
    slog("---==== END ====---");

    

    return 0;
}
