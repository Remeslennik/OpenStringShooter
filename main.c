//Russian: Заготовка для игры, по дорожке бежит противник, нужно нажать 123456 для стрельбы. Пробел - рестарт
// Используйте акустическую гитару и микрофон, дерните струну, вылетит пуля
// форум проекта http://www.gamedev.ru/code/forum/?id=183756

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <time.h>
#include <math.h>
#include "portaudio.h"

// для портаудио
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (4096)
#define POW_2 (12)
#define NUM_CHANNELS (1)
#define PA_SAMPLE_TYPE paFloat32 // Select sample format
#define SAMPLE_SIZE (4)
#define SAMPLE_SILENCE (0.0f)
#define CLEAR(a) memset( (a), 0, FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE ) // обнуление массива
#define PRINTF_S_FORMAT "%.8f"
#define  NUMBER_IS_2_POW_K(x)   ((!((x)&((x)-1)))&&((x)>1))  // x is pow(2, k), k=1,2, ...
#define  FT_DIRECT        -1    // Direct transform.
#define  FT_INVERSE        1    // Inverse transform.

// аудио анализ
int FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag); // преобразование фурье
//extern int FFT();
void WIN_FUN(float *Index, int n); // коэффициенты оконной функции
extern float GetRMSpower(); // среднеквад мощность
//extern int GetOpenString(); // определение открытой струны, передаем спектр и размер
extern int GetOpenString(float* array_adr, int array_size);

// рисование
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
logic.bullet_speed=40; // глобал
logic.ghost_x=750;
logic.ghost_speed=1;
logic.miss=0;
logic.gameover=1;


srand( time( NULL ) ); // for random
logic.ghost_str=( rand()%(6))+1;

// init PortAudio ########################################################
PaStreamParameters inputParameters;
PaStream *stream = NULL;
float *sampleBlock;
int i, max_i;
int open_string=0;
int numBytes;
float Im[FRAMES_PER_BUFFER]; // мнимая часть
float PowFreq[FRAMES_PER_BUFFER]; //мощности частот
float WinFun[FRAMES_PER_BUFFER]; // оконная функция
int freq_first=25*FRAMES_PER_BUFFER/SAMPLE_RATE;// 50 нижняя частота
int freq_last=20000*FRAMES_PER_BUFFER/SAMPLE_RATE;// 700 верхняя частота
WIN_FUN(WinFun,FRAMES_PER_BUFFER); // вычисляем коэффициенты оконой функции
// вычисляем размер буфера
numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE ;
sampleBlock = (float *) malloc( numBytes );
CLEAR(sampleBlock);
// инициализация портаудио
Pa_Initialize();
inputParameters.device = Pa_GetDefaultInputDevice();
inputParameters.channelCount = NUM_CHANNELS;
inputParameters.sampleFormat = PA_SAMPLE_TYPE;
inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
inputParameters.hostApiSpecificStreamInfo = NULL;
// открытие потока
Pa_OpenStream(
    &stream,
    &inputParameters,
    0,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    paClipOff,
    NULL,
    NULL );
// старт потока
Pa_StartStream( stream );



// initialize SDL video ##################################################
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

// program main loop #################################################
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
                logic.bullet_speed=40;
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

    // START AUDIO ANALIZATOR
    Pa_ReadStream( stream, sampleBlock, FRAMES_PER_BUFFER ); // захват звука
    CLEAR( Im ); // обнуляем мнимую часть
    for (i=0; i<FRAMES_PER_BUFFER; ++i) sampleBlock[i]*=WinFun[i];// применяем оконную функцию
    FFT(sampleBlock, Im, FRAMES_PER_BUFFER, POW_2, -1); // отправляем на фурье
    for (i=freq_first; i<freq_last; ++i) PowFreq[i]=sampleBlock[i]*sampleBlock[i]+Im[i]*Im[i]; // квадраты мощностей
    open_string= GetOpenString(PowFreq,FRAMES_PER_BUFFER); // ищем открытую струну, иначе 0
    // выстрел при звуке
    if (open_string==1)
    {
        GunString=1;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=1, logic.bullet_x=150;
    }
    if (open_string==2)
    {
        GunString=2;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=2, logic.bullet_x=150;
    }
    if (open_string==3)
    {
        GunString=3;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=3, logic.bullet_x=150;
    }
    if (open_string==4)
    {
        GunString=4;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=4, logic.bullet_x=150;
    }
    if (open_string==5)
    {
        GunString=5;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=5, logic.bullet_x=150;
    }
    if (open_string==6)
    {
        GunString=6;
        if (logic.shoot==0) logic.shoot=1, logic.bullet_str=6, logic.bullet_x=150;
    }


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
    SDL_Delay(5);
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
        if (logic.ghost_x > 450)
        {
            logic.ghost_speed+=1; // стрелок слишком быстрый
            logic.bullet_speed=40+logic.ghost_speed*4;
        }
        logic.ghost_x=720;
    }

}
}


void WIN_FUN(float *Index, int n)
{
// вычисляем коэффициенты оконной функции
int i;
double a;
for (i=0; i<n ; i++)
{
    a = M_PI / n * (1.0 + 2.0 * i);
    Index[i]=0.5 * (1.0 - 0.16 - cos(a) + 0.16 * cos(2.0 * a));
}
}

// преобразование фурье, код с вики
int  FFT(float *Rdat, float *Idat, int N, int LogN, int Ft_Flag)
{
// parameters error check:
if((Rdat == NULL) || (Idat == NULL))                  return 0;
if((N > 16384) || (N < 1))                            return 0;
if(!NUMBER_IS_2_POW_K(N))                             return 0;
if((LogN < 2) || (LogN > 14))                         return 0;
if((Ft_Flag != FT_DIRECT) && (Ft_Flag != FT_INVERSE)) return 0;

register int  i, j, n, k, io, ie, in, nn;
float         ru, iu, rtp, itp, rtq, itq, rw, iw, sr;

static const float Rcoef[14] =
{
    -1.0000000000000000F,  0.0000000000000000F,  0.7071067811865475F,
    0.9238795325112867F,  0.9807852804032304F,  0.9951847266721969F,
    0.9987954562051724F,  0.9996988186962042F,  0.9999247018391445F,
    0.9999811752826011F,  0.9999952938095761F,  0.9999988234517018F,
    0.9999997058628822F,  0.9999999264657178F
};
static const float Icoef[14] =
{
    0.0000000000000000F, -1.0000000000000000F, -0.7071067811865474F,
    -0.3826834323650897F, -0.1950903220161282F, -0.0980171403295606F,
    -0.0490676743274180F, -0.0245412285229122F, -0.0122715382857199F,
    -0.0061358846491544F, -0.0030679567629659F, -0.0015339801862847F,
    -0.0007669903187427F, -0.0003834951875714F
};

nn = N >> 1;
ie = N;
for(n=1; n<=LogN; n++)
{
    rw = Rcoef[LogN - n];
    iw = Icoef[LogN - n];
    if(Ft_Flag == FT_INVERSE) iw = -iw;
    in = ie >> 1;
    ru = 1.0F;
    iu = 0.0F;
    for(j=0; j<in; j++)
    {
        for(i=j; i<N; i+=ie)
        {
            io       = i + in;
            rtp      = Rdat[i]  + Rdat[io];
            itp      = Idat[i]  + Idat[io];
            rtq      = Rdat[i]  - Rdat[io];
            itq      = Idat[i]  - Idat[io];
            Rdat[io] = rtq * ru - itq * iu;
            Idat[io] = itq * ru + rtq * iu;
            Rdat[i]  = rtp;
            Idat[i]  = itp;
        }

        sr = ru;
        ru = ru * rw - iu * iw;
        iu = iu * rw + sr * iw;
    }

    ie >>= 1;
}

for(j=i=1; i<N; i++)
{
    if(i < j)
    {
        io       = i - 1;
        in       = j - 1;
        rtp      = Rdat[in];
        itp      = Idat[in];
        Rdat[in] = Rdat[io];
        Idat[in] = Idat[io];
        Rdat[io] = rtp;
        Idat[io] = itp;
    }

    k = nn;

    while(k < j)
    {
        j   = j - k;
        k >>= 1;
    }

    j = j + k;
}

if(Ft_Flag == FT_DIRECT) return 1;

rw = 1.0F / N;

for(i=0; i<N; i++)
{
    Rdat[i] *= rw;
    Idat[i] *= rw;
}

return 1;
}

float interp_line(float x1, float y1, float x2, float y2, float x);

int GetOpenString(float* array_adr, int array_size)
{
int note1=64, note2=59, note3=55, note4=50, note5=45, note6=40;// номера нот струн

float freq1 = 440 * powf(2, (note1-69)/12.) ; // номер ноты в частоту
float freq2 = 440 * powf(2, (note2-69)/12.) ;
float freq3 = 440 * powf(2, (note3-69)/12.) ;
float freq4 = 440 * powf(2, (note4-69)/12.) ;
float freq4_2 = 440 * powf(2, (note4+2-69)/12.) ;
float freq5 = 440 * powf(2, (note5-69)/12.) ;
float freq6 = 440 * powf(2, (note6-69)/12.) ;

// индексы частот в спектре
// Freq_1=X0*SAMPLE_RATE/FRAMES_PER_BUFFER; // переводим в частоту
float x1 = freq1* array_size /44100;
float x2 = freq2* array_size /44100;
float x3 = freq3* array_size /44100;
float x4 = freq4* array_size /44100;
float x4_2 = freq4_2* array_size /44100;
float x5 = freq5* array_size /44100;
float x6 = freq6* array_size /44100;

//  наличие частоты в спектре, линейной интерполяцией
float pow1 = interp_line ( floor(x1), // округление для типа float
                           sqrt(array_adr[(int)x1]), // округление для индекса int
                           floor(x1)+1,
                           sqrt(array_adr[(int)x1+1]),
                           x1);

float pow2 = interp_line ( floor(x2),
                           sqrt(array_adr[(int)x2]),
                           floor(x2)+1,
                           sqrt(array_adr[(int)x2+1]),
                           x2);
float pow3 = interp_line ( floor(x3),
                           sqrt(array_adr[(int)x3]),
                           floor(x3)+1,
                           sqrt(array_adr[(int)x3+1]),
                           x3);
float pow4 = interp_line ( floor(x4),
                           sqrt(array_adr[(int)x4]),
                           floor(x4)+1,
                           sqrt(array_adr[(int)x4+1]),
                           x4);
float pow4_2 = interp_line ( floor(x4_2),
                             sqrt(array_adr[(int)x4_2]),
                             floor(x4_2)+1,
                             sqrt(array_adr[(int)x4_2+1]),
                             x4_2);
float pow5 = interp_line ( floor(x5),
                           sqrt(array_adr[(int)x5]),
                           floor(x5)+1,
                           sqrt(array_adr[(int)x5+1]),
                           x5);
float pow6 = interp_line ( floor(x6),
                           sqrt(array_adr[(int)x6]),
                           floor(x6)+1,
                           sqrt(array_adr[(int)x6+1]),
                           x6);


// определяем струну
int curentstring=0;
if (pow1 > 100 ) curentstring = 1;
if (pow2 > 100 && pow6 <5) curentstring = 2;
if (pow3 > 100 ) curentstring = 3;
if (pow4 > 70 ) curentstring = 4;
if (pow5 > 100 ) curentstring = 5;
if (pow6 > 50 ) curentstring = 6;
if (pow4_2 > 100  && pow2>50) curentstring = 6;
return (curentstring);
}

float interp_line (float x1, float y1, float x2, float y2, float x)
{

float y = y1 + ((y2-y1)*(x-x1) / (x2-x1)); // интерполяция по двум точкам
return (y);
}
