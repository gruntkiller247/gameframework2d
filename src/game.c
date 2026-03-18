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
#include "MattHelper.h"
#include "bomb.h"
#include "powerup.h"

#define MY_FONT "fonts/FreeSans.ttf"

//128 x 128 grid for GIMP + snap to grid

void drawUI(Entity* player, Entity* boss, TTF_Font* font)
{
    if (!font)
    {
        return;
    }

    //SDL_RenderClear(gf2d_graphics_get_renderer());
    //SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Test Text", color);


    if (!textSurface)
    {
        slog("Text render failed: %s\n", TTF_GetError());
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), textSurface);

    if (!textTexture)
    {
        slog("Failed to create texture: %s\n", SDL_GetError());
        return;
    }

    GFC_Rect menu;
    menu.x = 100;
    menu.y = 100;
    menu.w = textSurface->w;
    menu.h = textSurface->h;

    SDL_RenderCopy(gf2d_graphics_get_renderer(), textTexture, NULL, &menu);
    //SDL_RenderPresent(gf2d_graphics_get_renderer());
    //SDL_Delay(2000);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    //gf2d_draw_rect_filled(menu, GFC_COLOR_WHITE);
    return;
}

    /*if (!player)
        ;
    else
    {
        //Draw Player's HP

    }

    if (!boss)
        ;
    else
    {
        //Draw Boss's HP
    }

    //SDL_FreeSurface(textSurface);
    //SDL_DestroyTexture(textTexture);
    //TTF_Quit();
    
    return;
}*/

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
    //slog("The curret time is %i",time(NULL));


    //SDL_GetTikcs returns milli seconds program has been running
    
    /*program initializtion*/
    init_logger("gf2d.log",0);
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "gf2d",
        1200,
        720,
        1200,
        720,
        gfc_vector4d(0,0,0,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gf2d_sprite_init(1024);

    if (TTF_Init() == -1) {
        slog("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        slog("Couldn't initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    TTF_Font* font = TTF_OpenFont(MY_FONT, 64);

    entityManagerInit(2048);
    //monsterManagerInit(1024);

    //Not Working?
    gfc_input_init("config/input.gfc");

    SDL_ShowCursor(SDL_DISABLE);
    
    /*demo setup*/
    sprite = gf2d_sprite_load_image("images/backgrounds/cat.jpg");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    slog("press [escape] to quit");

 
    

    Entity* player;
    player = playerEntityNew(gfc_vector2d(0, 0), ROLE_PLAYER_BAKER);
    strcpy(player->name, "Player");

    Entity* boss = NULL;


    int level = 1;

    switch (level)
    {
         case  1:
             /*Entity* projectile;
                projectile = projectileEntityNew(gfc_vector2d(300, 0), TEAM_ENEMY, -1);
                projectile->team = TEAM_IGNORE;
                strcpy(projectile->name, "TEST_PROJECTILE");*/

             Entity * enemy;
             enemy = monsterEntityNew(gfc_vector2d(100, 100));
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

             break;

         default:
             slog("No Level Loaded!");
    }

   
    
    /*main game loop*/
    while(!done)
    {
       
        gfc_input_update();
        //SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;

        
        //update Thinking Here
        entityThinkAll();


        
        
        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            gf2d_sprite_draw_image(sprite,gfc_vector2d(0,0));
            
            
            entityManagerDrawAll();
            

            //UI elements last
            gf2d_sprite_draw(
                mouse,
                gfc_vector2d(mx,my),
                NULL,
                NULL,
                NULL,
                NULL,
                &mouseGFC_Color,
                (int)mf);

            //gf2d_draw_line(gfc_vector2d(0,0), gfc_vector2d(100,100), GFC_COLOR_RED);



            entityTouchAll();
            entityUpdateAll();
            entityFreeAll();

            
            drawUI(player, boss, font);
           

            //SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);




        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        
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


    TTF_CloseFont(font);
    TTF_Quit();
    entityManagerClose();
    //slog("entityManager is closed!");
    //monsterManagerClose();
    slog("---==== END ====---");

    

    return 0;
}
