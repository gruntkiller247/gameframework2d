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

void updateUI(Uint8 playerNumHP, Uint8 bossNumHP, TTF_Font* font, SDL_Color color, SDL_Rect** playerHPUI, SDL_Rect** bossHPUI)
{
    if (!font || !playerHPUI)
    {
        slog("Font or player UI not working! Can't draw!");
        return;
    }
    //slog("Player hp is %i", player);
        

    snprintf(playerHP, sizeof(playerHP), "Player HP: %i", playerNumHP);
    snprintf(bossHP, sizeof(bossHP), "Boss HP: %i", bossNumHP);

    surface = TTF_RenderText_Solid(font, playerHP, color);
    texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
    dstRect.x = 10;
    dstRect.y = 10;
    dstRect.w = surface->w / 2;
    dstRect.h = surface->h / 2;
    SDL_FreeSurface(surface);

    if (!bossHPUI)
    {
        //slog("No Boss for UI to draw!");
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
   

    int paused = 0;
    

    surfaceBoss = TTF_RenderText_Solid(font, bossHP, color);
    textureBoss = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceBoss);


    Entity* boss = NULL;
    Entity* player = NULL;

    int level = 1;
    int numPowerUps = 0;
    int powerUpSpawning = 0;
    int powerUpCounter = 0;
    int powerUpTime = 500;
    Entity* powerUpGame = NULL;

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
    
    
    entityManagerInit(2048);
    gfc_input_init("config/input.gfc");
    SDL_ShowCursor(SDL_DISABLE);
    

    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/cat.jpg");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");

    player = playerEntityNew(gfc_vector2d(0, 0), ROLE_PLAYER_BAKER);
    strcpy(player->name, "Player");

    
    level = 2;

    switch (level)
    {
         case  1:
             slog("Loading Testing Level");
             /*Entity* projectile;
                projectile = projectileEntityNew(gfc_vector2d(300, 0), TEAM_ENEMY, -1);
                projectile->team = TEAM_IGNORE;
                strcpy(projectile->name, "TEST_PROJECTILE");*/

             Entity * enemy;
             enemy = monsterEntityNew(gfc_vector2d(100, 100),ROLE_TRASHMOB);
             strcpy(enemy->name, "Mr Monster!");
             enemy->hp = 10;

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
             powerUpSpawning = 1;

             boss = monsterEntityNew(gfc_vector2d(100, 100),ROLE_BOSS1);
             strcpy(boss->name, "POS");
             break;

         default:
             slog("No Level Loaded!");
    }

    snprintf(playerHP, sizeof(playerHP), "Player HP: %d", player->hp);
    surface = TTF_RenderText_Solid(font, playerHP, color);
    texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);


    dstRect.x = 10;
    dstRect.y = 10;
    dstRect.w = surface->w/2;
    dstRect.h = surface->h/2;
    SDL_FreeSurface(surface);

    if (!boss)
    {
        ;
    }
    else
    {
        snprintf(bossHP, sizeof(bossHP), "Boss HP: %d", boss->hp);
        
        surfaceBoss = TTF_RenderText_Solid(font, bossHP, color);
        textureBoss = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceBoss);

       
        dstRect2.x = 1000;
        dstRect2.y = 10;
        dstRect2.w = surfaceBoss->w / 2;
        dstRect2.h = surfaceBoss->h / 2;
        SDL_FreeSurface(surfaceBoss);
    }

   
    
    /*main game loop*/
    while(!done)
    {
       
        gfc_input_update();
        //SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);

        if (!paused)
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
                    numPowerUps++;
                }

            }
            else if (numPowerUps == 3)
            {
                slog("Power ups spawns maxed out!");
            }
            else
            {
                ;
            }



            //update Thinking Here
            entityThinkAll();




            gf2d_graphics_clear_screen();// clears drawing buffers
            // all drawing should happen betweem clear_screen and next_frame
                //backgrounds drawn first
            gf2d_sprite_draw_image(sprite, gfc_vector2d(0, 0));


            entityManagerDrawAll();


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
            entityUpdateAll();
            entityFreeAll();
            entityBoundsCheckAll();


            updateUI(player->hp, boss->hp, font, color, &dstRect, &dstRect2);

            SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);


            //Render text
            SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &dstRect);
            SDL_RenderCopy(gf2d_graphics_get_renderer(), textureBoss, NULL, &dstRect2);
            SDL_RenderPresent(gf2d_graphics_get_renderer());


            gf2d_graphics_next_frame();// render current draw frame and skip to the next frame

        }
        else
        {
            if (gfc_input_key_pressed("g"))
            {
                slog("Unpausing?");
                paused = 0;
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

            
            gf2d_graphics_next_frame();
              
        }
       
        if (keys[SDL_SCANCODE_ESCAPE])done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }

    //entityKillAll();

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
