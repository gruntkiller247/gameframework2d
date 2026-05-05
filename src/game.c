#include <SDL.h>
#include "simple_logger.h"
#include <SDL_ttf.h>
#include "UI.h"
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
#include "button.h"

#define MY_FONT "fonts/FreeSans.ttf"




//Static bullshit
//Window Size
static int viewWidth = 1200;
static int viewHeight = 720;
static int renderWdith = 1200;
static int renderHeight = 720;

Entity* bossGame = NULL;
Entity* player = NULL;

void fillEntityManager()
{
    int c;
    Entity* thing;

    for (c = 0; c < 5000; c++)
    {

            thing = bombEntityNew(gfc_vector2d(100, 100),TEAM_PLAYER,0);
            //thing = monsterEntityNew(gfc_vector2d(100,100),ROLE_TRASHMOB);
            //thing = projectileEntityNew(gfc_vector2d(100, 100), TEAM_PLAYER, -1, ROLE_PROJECTILE);
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
    int mx2, my2;
    float mf = 0;
    Sprite *mouse;
    GFC_Color mouseGFC_Color = gfc_color(1, 1, 1, 100);//= gfc_color8(255,100,255,200);

    srand(time(NULL));
    TTF_Font* font = NULL;
    SDL_Color color = { 255, 255, 255, 255 };
    Uint8 playerRole = ROLE_PLAYER_BAKER;

    int paused = NOT_PAUSED;
    
    Level* currentLevel;

    int level = -1;
    int numPowerUps = 0;
    int powerUpSpawning = 0;
    int powerUpCounter = 0;
    int powerUpTime = 500;
    Entity* powerUpGame = NULL;

    int distance;
    int functionSlogs = 1;
    int click = 0;


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
    SDL_Init(SDL_INIT_AUDIO);

    if (SDL_Init(SDL_INIT_AUDIO) == -1)
    {
        slog("Failed to initiate audio!");
        return 1;
    }



    
    initalizeLevel();
    entityManagerInit(16384);//16384);//2^14
    uiManagerInit(256); //2^8
    //levelManagerInit(32);


    gfc_input_init("config/input.gfc");
    SDL_ShowCursor(SDL_DISABLE);
    

    /*demo setup*/
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);

    if (!mouse)
    {
        slog("Failed to load mouse!");
    }

    slog("press [escape] to quit");

    
    currentLevel = dataLoadLevel("levels/mainMenu.level");

    if (!currentLevel)
    {
        slog("No level could be loaded! Loading error handling level!");
        
        currentLevel = dataLoadLevel("levels/mainMenu.level"); 
        sprite = gf2d_sprite_load_image(currentLevel->background);
    }
    else
    {
        slog("Level Loaded and not NULL!");

        if (!currentLevel->background)
        {
            slog("Level Loaded without a background Image! Giving default!");
            currentLevel->background = "images/backgrounds/bg_flat.png";
        }
        sprite = gf2d_sprite_load_image(currentLevel->background);
        if (currentLevel->spawnPowerUps)
            powerUpSpawning = 1;
    }
    

    player = getPlayer();
    bossGame = getBoss();

    int testNum = 0;


    /*main game loop*/
    while(!done)
    {
        //slog("Num elements in gfc_list: %i", gfc_list_get_count(currentLevel->levelUI));
        gfc_input_update();
        //SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);

        

        if (getLevelStatus() == LS_END_GAME)
        {
            done = 1;
            slog("Ending the Game!");
            break;
        }
        else if (getLevelStatus() == LS_NEW_LEVEL)
        {
            slog("MAIN GAME LOOP! NEW LEVEL NEEDS TO BE LOADED!");
            
            //Kill everything - ONLY THE UI AND ENTITIES
            entityKillAll();
            uiKillAll();
            slog("Next level is: %s", getNextLevel());
            currentLevel = dataLoadLevel(getNextLevel());

            slog_sync();



            setNextLevel(NULL);

            //currentLevel = getCurrentLevel();
            setLevelStatus(LS_NORMAL);
            
        }
        else if (!currentLevel)
        {
            //slog("CURRENT LEVEL IS NULL!");
        }
        else if (player && isPaused() == NOT_PAUSED && strcmp(currentLevel->name, "Custom Template") != 0)
        {


            if (gfc_input_key_pressed("g"))
            {
                slog("Pausing?");
                setPausedUI();
                //setPausedEntity();
            }
                

            mf += 0.1;
            if (mf >= 16.0)mf = 0;


            if (powerUpSpawning == 1/* && numPowerUps < 3*/)
            {
                if (rand() % 10000 == 1)
                {
                    slog("Spawning a power up!");
                    powerUpGame = powerUpEntityNew(gfc_vector2d(rand() % 1200, rand() % 720), ROLE_PU_RANDOM);
                    //numPowerUps++;
                }

            }
            
            

            //update Thinking Here
            entityThinkAll();
            //slog("Thunk");


            gf2d_graphics_clear_screen();// clears drawing buffers
            // all drawing should happen betweem clear_screen and next_frame
                //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));

            levelDraw(currentLevel);

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
            uiTouchAll();
            

            entityUpdateAll();
            //slog("Updated");
            uiUpdateAll();
            

            entityBoundsCheckAll();

            entityManagerDrawAll();
            uiDrawAll();
            
            
            //slog("Bounds checked!");

            

            entityFreeAll();
            uiFreeAll();
            
           
           
            SDL_RenderPresent(gf2d_graphics_get_renderer());
            SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);

            gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
            //slog("1 cycle!");
        }
        else if(player && isPaused() == PAUSED && strcmp(currentLevel->name, "Custom Template") != 0)
        {

            
            if (gfc_input_key_pressed("g"))
            {
                slog("Unpausing?");
                setPausedUI();
            } 

            if (gfc_input_key_pressed("1"))
            {
                player->hp = 0;
            }

            if (gfc_input_key_pressed("2"))
            {
                slog("Player Points: %i", getPlayerPoints());
            }

            gf2d_graphics_clear_screen();
            levelDraw(currentLevel);
            //gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));
            entityManagerDrawAll();
            uiUpdateAll();
            uiTouchAll();
            uiDrawAll();

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
              
        }
        else
        {
           //In the level builder!
           //Mouse should be a placer: Buttons should toggle!
           //Make button at bottom of screen/buttons on keyboard
           //Click button -> Mouse click places dude!


           if (isSpawning() == S_IS)
           {
               //Spawn that thing!
               //slog("IS_SPAWNING TRUE!");
               //slog("Role to spawn: %i", roleToSpawn());
               if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK && !click)
               {
                   click = 1;
                   mx2 = mx;
                   my2 = my;
                    //I have clicked a button, thus I have something I want to spawn elsewhere!
                   slog("Click = 1");
               }
               else if ((mx2 != mx && my!= my2) && SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK && click == 1)
               {
                   slog("Clicked elsewhere!");
                   //I have clicked another spot, thus spawn the thing here!
                   switch (roleToSpawn())
                   {
                   case ROLE_TRASHMOB:


                   case ROLE_BOSS1:


                   case ROLE_BOSS2:


                   case ROLE_BOSS3:

                       monsterEntityNew(gfc_vector2d(mx, my), roleToSpawn());
                       clearSpawning();
                       break;

                   default:
                       slog("Nothing to spawn!");
                       clearSpawning();
                      

                   }
                   click = 0;
                   mx2 = 0;
                   my2 = 0;
               }
           }
           else
           {
               //There is nothing to spawn here!
           }

          
           //entityThinkAll();

           gf2d_graphics_clear_screen();// clears drawing buffers
              
           gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));

           

           //entityTouchAll();
               
           uiTouchAll();


           //entityUpdateAll();
           
           uiUpdateAll();

           levelDraw(currentLevel);

           //entityBoundsCheckAll();

           entityManagerDrawAll();
           uiDrawAll();


           entityFreeAll();
           uiFreeAll();

           gf2d_sprite_draw(mouse, gfc_vector2d(mx, my), NULL, NULL, NULL, NULL, &mouseGFC_Color, (int)mf);

             
           SDL_RenderPresent(gf2d_graphics_get_renderer());
           SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);

           gf2d_graphics_next_frame();


        }

        if (player->hp <= 0)
        {
            slog("The player is dead! Load main menu!");
            entityKillAll();
            uiKillAll();
            currentLevel = dataLoadLevel("levels/mainMenu.level");
            sprite = gf2d_sprite_load_image(currentLevel->background);
            setPlayer(getPlayer());
            player = getPlayer();
            setPlayerPoints(0);
        }

        if (keys[SDL_SCANCODE_ESCAPE])
            done = 1; // exit condition
    }
    
    if (strcmp(currentLevel->name, "Custom Template") == 0)
    {
        saveLevel();
    }
    
    SDL_CloseAudio();
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();
    slog("---==== END ====---");
    
    return 0;
}
