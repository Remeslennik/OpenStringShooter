//Russian: Заготовка для игры, по дорожке бежит противник, нужно нажать 123456 для стрельбы. Пробел - рестарт
// В будущем будет приделан анализатор звука с микрофона

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <time.h>

void DrawTrack(); // процедуры рисования
void DrawGun(int GunString);
void DrawGhost();
void DrawBullet();
void Collision();

SDL_Surface *screen; // поверхности
SDL_Surface *GUN  =  IMG_Load ("pic/gun.png");
SDL_Surface *GHOST  =  IMG_Load ("pic/ghost.png");
SDL_Surface *BULLET  =  IMG_Load ("pic/bullet.png");

SDL_Rect gun; // координаты
SDL_Rect ghost;
SDL_Rect bullet;

struct LOGIC // структура для игровой логики
{
    int bullet_str; // по струне определяют y
    int bullet_x;
    int shoot;
    float bullet_speed;
    int ghost_str;
    int ghost_x;
    float ghost_speed;
    int miss;
    int gameover;
} logic;


int main ( int argc, char** argv )
{
    int GunString=1;// локал
    logic.bullet_speed=15; // глобал
    logic.ghost_x=750;
    logic.ghost_speed=0.1;
    logic.miss=0;
    logic.gameover=1;


    srand( time( NULL ) ); // for random
    logic.ghost_str=( rand()%(6))+1;
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    screen = SDL_SetVideoMode(800, 600, 32,
                              SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Unable to set 800x600 video: %s\n", SDL_GetError());
        return 1;
    }

    // program main loop
    int done = 0;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
            {
                // exit if ESCAPE is pressed
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    done = 1;
                    break;
                }
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    logic.gameover = 0;
                    logic.shoot=0; // чтоб пуля не летела при старте
                    logic.miss=0;
                    logic.ghost_speed=1;
                }
                if (event.key.keysym.sym == SDLK_1)
                {
                    GunString=1;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=1, logic.bullet_x=150;
                }
                if (event.key.keysym.sym == SDLK_2)
                {
                    GunString=2;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=2, logic.bullet_x=150;
                }
                if (event.key.keysym.sym == SDLK_3)
                {
                    GunString=3;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=3, logic.bullet_x=150;
                }
                if (event.key.keysym.sym == SDLK_4)
                {
                    GunString=4;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=4, logic.bullet_x=150;
                }
                if (event.key.keysym.sym == SDLK_5)
                {
                    GunString=5;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=5, logic.bullet_x=150;
                }
                if (event.key.keysym.sym == SDLK_6)
                {
                    GunString=6;
                    if (logic.shoot==0) logic.shoot=1, logic.bullet_str=6, logic.bullet_x=150;
                }
            }
            } // end switch
        } // end of message processing

        // DRAWING STARTS HERE

        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 255, 255, 255));

        DrawTrack(); //рисуем дорожки
        DrawGun(GunString);// пушка
        DrawGhost(); // враги
        if (logic.gameover==1)
        {
            SDL_Flip(screen);
            SDL_Delay(10);
            continue;
        }




        DrawBullet(); // пуля
        Collision(); // расчет попаданий

        // DRAWING ENDS HERE

        // finally, update the screen :)
        SDL_Flip(screen);
        //delay
        SDL_Delay(10);
    } // end main loop

    // free loaded bitmap
    SDL_FreeSurface(GUN);
    SDL_FreeSurface(BULLET);
    SDL_FreeSurface(GHOST);

    return 0;
}

void DrawTrack()
{
    SDL_Rect track;
    track.x=160;
    track.y=150;
    track.w=600;
    track.h=50;
    SDL_SetClipRect (screen, &track); // ограничение прямоугольника для дорожки
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0xe1, 0x5c, 0xec));//1
    track.y+=60;
    SDL_SetClipRect (screen, &track);
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0x30, 0xc6, 0x41));//2
    track.y+=60;
    SDL_SetClipRect (screen, &track);
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0xd7, 0x98, 0x30));//3
    track.y+=60;
    SDL_SetClipRect (screen, &track);
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0x49, 0xa8, 0xd7));//4
    track.y+=60;
    SDL_SetClipRect (screen, &track);
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0xf4, 0xdd, 0x23));//5
    track.y+=60;
    SDL_SetClipRect (screen, &track);
    SDL_FillRect (screen,0,SDL_MapRGB(screen->format, 0xfb, 0x27, 0x1d));//6
    SDL_SetClipRect (screen, 0);//область рисования - вся поверхность
}

void DrawGun(int GunString)
{
    gun.y=150+60*(GunString-1);
    SDL_BlitSurface(GUN, 0, screen, &gun);
}

void DrawGhost()
{
    if (!logic.gameover)
    {
        ghost.y=151+60*(logic.ghost_str-1);
        ghost.x=(int) logic.ghost_x;
        SDL_BlitSurface(GHOST, 0, screen, &ghost);
        logic.ghost_x-=(int)logic.ghost_speed;
        if ( logic.ghost_x <=150)
        {
            logic.ghost_x=720;
            logic.miss++;
            //logic.ghost_speed+=1; // для бесконечного ускорения
            if (logic.miss==3) logic.gameover=1;
        }
    }
    // рисуем промахи
    int i;
    ghost.y=20;
    if (logic.miss>0) // есть пропуски
    {
        for (i=0; i<logic.miss; i++)
        {
            ghost.x=20+60 * (i);
            SDL_BlitSurface(GHOST, 0, screen, &ghost);
        }
    }
}

void DrawBullet()
{
    if(logic.shoot==1)
    {
        bullet.y=165+60*(logic.bullet_str-1);
        bullet.x=(int)logic.bullet_x;
        SDL_BlitSurface(BULLET, 0, screen, &bullet);
        logic.bullet_x+=logic.bullet_speed;
        if (logic.bullet_x>=760)
        {
            logic.shoot=0;
        }
    }
}

void Collision()
{
    if (logic.bullet_str == logic.ghost_str)
    {
        if (logic.bullet_x > logic.ghost_x)
        {
            logic.shoot=0;
            logic.bullet_x=0;
            logic.ghost_str=( rand()%(6))+1;
            if (logic.ghost_x > 450) logic.ghost_speed+=0.1; // стрелок слишком быстрый
            logic.ghost_x=720;
        }

    }
}

