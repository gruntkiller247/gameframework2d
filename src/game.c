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

//Static bullshit for window sizes
static int viewWidth = 1200;
static int viewHeight = 720;
static int renderWdith = 1200;
static int renderHeight = 720;

//This code was stolen from some fucking person on stack overflow and it works for everyone but me!
//https://stackoverflow.com/questions/22886500/how-to-render-text-in-sdl2
void stolenCodeThatClaimsToWorkButDoesNot()
{
    //this opens a font style and sets a size
    TTF_Font* Sans = TTF_OpenFont("Sans.ttf", 24);

    // this is the color in rgb format,
    // maxing out all would give you the color white,
    // and it will be your text's color
    SDL_Color White = { 255, 255, 255 };

    // as TTF_RenderText_Solid could only be used on
    // SDL_Surface then you have to create the surface first
    SDL_Surface* surfaceMessage =
        TTF_RenderText_Solid(Sans, "put your text here", White);

    // now you can convert it into a texture
    SDL_Texture* Message = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surfaceMessage);

    SDL_Rect Message_rect; //create a rect
    Message_rect.x = 0;  //controls the rect's x coordinate 
    Message_rect.y = 0; // controls the rect's y coordinte
    Message_rect.w = 100; // controls the width of the rect
    Message_rect.h = 100; // controls the height of the rect

    // (0,0) is on the top left of the window/screen,
    // think a rect as the text's box,
    // that way it would be very simple to understand

    // Now since it's a texture, you have to put RenderCopy
    // in your game loop area, the area where the whole code executes

    // you put the renderer's name first, the Message,
    // the crop size (you can ignore this if you don't want
    // to dabble with cropping), and the rect which is the size
    // and coordinate of your texture
    SDL_RenderCopy(gf2d_graphics_get_renderer(), Message, NULL, &Message_rect);
    
    // Don't forget to free your surface and texture
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}

void realUI(Entity* player, Entity* boss, SDL_Surface* words)
{
    if (!words)
    {
        slog("Words are Null! Error with rendering text to surface!");
        return;
    }
        

}


//This function is meant to draw the HP bars for both the Boss, if any, and the Player dyanamically, but the stupid fucking code does not fucking work and I am sick of it!
//Instead, I manually mapped out the numbers and drew them with lines. Horseshit and should have never needed to be done!
void drawUI(Entity* player, Entity* boss, TTF_Font* font)
{
    if (!font)
    {
        slog("Font does not exist!");
        return;
    }

    double* wid, hei;
    SDL_Color color = { 255, 0, 0, 255 };
    SDL_Texture* textImage = NULL;
    SDL_Surface* surface = NULL;
    //SDL_Texture* Final = NULL;

    GFC_Rect menu;

    
    
    //SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 255, 255, 255, 255);
    //SDL_RenderClear(gf2d_graphics_get_renderer());


    surface = TTF_RenderText_Blended(font, "Test Text", color);

    if (!surface)
    {
        slog("Surface failed: %s\n", TTF_GetError());
        goto fail;
    }

    wid = surface->w;
    hei = surface->h;

    //surface = gf2d_graphics_screen_convert(&surface);

    if (!surface)
    {
        slog("Conversion lead to NULL!");
        goto fail;
    }
    
    textImage = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
    SDL_FreeSurface(surface);

    if (!textImage)
    {
        slog("Failed to make new texture Error:%s",SDL_GetError());
        goto fail;
    }
    
    

    menu.x = 0;
    menu.y = 0;
    menu.w = (int)wid;
    menu.h = hei;

    gf2d_draw_rect_filled(menu, GFC_COLOR_WHITE);
    SDL_RenderCopy(gf2d_graphics_get_renderer(), textImage, NULL, &menu);
    SDL_RenderPresent(gf2d_graphics_get_renderer());
    
    //SDL_Delay(100);

    //SDL_DestroyTexture(textImage);
    
    return;

    fail:
    
    if (!surface)
        ;
    else
        SDL_FreeSurface(surface);

    if (!textImage)
        ;
    else
        SDL_DestroyTexture(textImage);
;

    return;
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


    //slog("The curret time is %i",time(NULL));
    //SDL_GetTikcs returns milli seconds program has been running
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
    int powerUpSpawning = 0;
    int powerUpCounter = 0;
    int powerUpTime = 500;
    Entity* powerUpGame = NULL;

    switch (level)
    {
         case  1:
             slog("Loading Testing Level");
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
             powerUpSpawning = 1;
             break;

         default:
             slog("No Level Loaded!");
    }

    SDL_Color color = { 255, 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello SDL_ttf!", color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);

    SDL_Rect dstRect;
    dstRect.x = 100;
    dstRect.y = 100;
    dstRect.w = surface->w;
    dstRect.h = surface->h;
    SDL_FreeSurface(surface);
    
   
    
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


        if (powerUpSpawning == 1)
            powerUpGame = powerUpEntityNew(gfc_vector2d(rand() % 1200,rand() % 720), -1);

        
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


            //player->position = gfc_vector2d(100, 100);
            
            entityTouchAll();
            entityUpdateAll();
            entityFreeAll();
            entityBoundsCheckAll();
           
            //drawUI(player, boss, font);
            //realUI(player, boss, surface);
            SDL_SetRenderDrawColor(gf2d_graphics_get_renderer(), 0, 0, 0, 255);
            SDL_RenderClear(gf2d_graphics_get_renderer());

            // Render text
            SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &dstRect);

            SDL_RenderPresent(gf2d_graphics_get_renderer());


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
