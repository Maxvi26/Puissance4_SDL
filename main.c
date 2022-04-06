#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <unistd.h>
#include "board/raspberry/gpio.h"

#ifndef max
#define max(i,j) (i>j?i:j)
#endif
#ifndef _min
#define _min(i,j) (i<j?i:j)
#endif

#define GPIO_HAUT 26
#define GPIO_BAS 13
#define GPIO_DROITE 19
#define GPIO_GAUCHE 25


/***************************************init***************************/

typedef struct
{
    float x;
    float y;
} coordonnees;

typedef struct
{
    signed char rouge;
    signed char vert;
    signed char bleu;
} rgb;

typedef enum
{
    vide = 0,
    etat_cercle = 1,
    etat_croix = 2,
} etat_case;


void up(void);
void SDL_EcrireCouleur(SDL_Surface* surface, int x, int y, int r, int g, int b);
static void SDL_LireCouleur(const SDL_Surface * surface, int x, int y, Uint8 * r, Uint8 * g, Uint8 * b);
unsigned long SDL_LireCouleurAux(const SDL_Surface* surface, int x, int y);
signed short ouverture_ecran(signed short x, signed short y);
void goto_xy(signed short x, signed short y);
void down(void);
void croix(void);
void EcrireCouleur(SDL_Surface* surface, int x, int y);
void ligne(SDL_Surface* surf,coordonnees A,coordonnees B);
void disque(void);
void gpio_inito(void);

Uint8 rayon = 46;
coordonnees dstrect;
rgb * rvb=NULL;
SDL_Surface * bmp=NULL;
SDL_Surface * screen=NULL;
TTF_Font *font=NULL, *font1=NULL;
char crayon=0;
Gpio * gpio_touch;
Gpio * gpio_touchH;
Gpio * gpio_touchB;
Gpio * gpio_touchD;
Gpio * gpio_touchG;
Uint16 x = 0, y = 0;
etat_case tab_etatcase[42]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //0=case vide,1=croix,2=rond
Uint16 tab_x[42]={-300,-200,-100,0,100,200,300,-300,-200,-100,0,100,200,300,-300,-200,-100,0,100,200,300,-300,-200,-100,0,100,200,300,-300,-200,-100,0,100,200,300,-300,-200,-100,0,100,200,300};
Uint16 tab_y[42]={-300,-300,-300,-300,-300,-300,-300,-200,-200,-200,-200,-200,-200,-200,-100,-100,-100,-100,-100,-100,-100,0,0,0,0,0,0,0,100,100,100,100,100,100,100,200,200,200,200,200,200,200};
//coordonnées de la case 1 en bas à gauche x=-300, y = -300

/************************************************************************************************************/

/***************************************MAIN***************************/
int main ( int argc, char** argv )
{

    Uint8 temp;
    char case_valid=0;
    gpio_inito();
    int retour = 0;
    Uint8 i=0;
    Uint8 bp_valid = 0;
    retour=ouverture_ecran(1000,800);

    if (retour==0) /* ouverture de l'écran réussi */
    {
        Uint8 dist = 100;
        up(); //rectangle extérieur
        goto_xy(-300,300);
        down();
        goto_xy(400,300);
        goto_xy(400,-300);
        goto_xy(-300,-300);
        goto_xy(-300,300);




        for(Uint8 i=0;i!=6;i++) //colonnes
        {

            up();
            goto_xy(-300+(i+1)*dist,300);
            down();
            goto_xy(-300+(i+1)*dist,-300);

        }

        for(Uint8 u=0;u!=5;u++) //lignes
        {

            up();
            goto_xy(-300,300-(u+1)*dist);
            down();
            goto_xy(400,300-(u+1)*dist);

        }

        up();
        x=0;
        y=200;
        goto_xy(x,y);
        while(1)
        {
            sleep(0.5);
            if(gpio_get_val(gpio_touchG)==0)
            {
                sleep(0.5);
                gauche_100();
            }
            if(gpio_get_val(gpio_touchD)==0)
            {
                sleep(0.5);
                droite_100();
            }
            if(gpio_get_val(gpio_touch)==1)
            {
                if(bp_valid==0)
                {
                    for(Uint8 a=0;a<7;a++){
                        if(x==((a*100)-300)){
                            for(Uint8 b=0;b<6;b++){
                                if(!(tab_etatcase[a+b*7]==(etat_cercle||etat_croix))){
                                    case_valid = 1;
                                    temp=a+b*7;
                                }
                            }

                            }

                        }
                        if(case_valid){
                                tab_etatcase[temp]=etat_cercle;
                                up();
                                goto_xy(tab_x[temp],tab_y[temp]);
                                x=tab_x[temp];
                                y=tab_y[temp];
                                disque();
                                case_valid = 0;

                    }
                    sleep(0.4);//tempo car sinon l'affichage disparait
                    bp_valid = 1;
                }
                else
                {

                    for(Uint8 a=0;a<7;a++){
                        if(x==((a*100)-300)){
                            for(Uint8 b=0;b<6;b++){
                                if(!(tab_etatcase[a+b*7]==(etat_cercle||etat_croix))){
                                    case_valid = 1;
                                    temp=a+b*7;
                                }
                            }

                            }

                        }
                        if(case_valid){
                                tab_etatcase[temp]=etat_cercle;
                                up();
                                goto_xy(tab_x[temp],tab_y[temp]);
                                x=tab_x[temp];
                                y=tab_y[temp];
                                croix();
                                case_valid = 0;

                    }
                    bp_valid = 0;
                }
            up();
            y=200;
            x=0;
            goto_xy(0,200);
            }
        }

    }
    return 0;
}


/************************************************************************************************************/

/*****************************************FONCTION****************************************************************/

signed short ouverture_ecran(signed short x, signed short y)
{
    SDL_Rect dstrect_SDL;


    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Incapable d'initier SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    screen = SDL_SetVideoMode(x, y, 16,SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !screen )
    {
        printf("Incapable d'ouvrir une fenetre video de %hdx%hd video : %s\n",x,y, SDL_GetError());
        return 1;
    }

	if( TTF_Init() == -1 )
	{
	    printf("Erreur init ttf");
		return 6;
	}
 	font = TTF_OpenFont( "/home/pi/joy-pi/P4_SDL/ChineseDragon.ttf", 20 );
	font1= TTF_OpenFont( "/home/pi/joy-pi/P4_SDL/ChineseDragon.ttf", 24 );
	if (font==NULL || font1==NULL)
    {
        printf("Incapable d'ouvrir la police de caractere : %s\n",SDL_GetError());
    }


    // load an image
    /*bmp = SDL_LoadBMP("p4image.bmp");
    if (!bmp)
    {
        printf("Incapable d'ouvrir l'image : %s\n", SDL_GetError());
        return 1;
    }

    // centre the bitmap on screen
    dstrect.x = (screen->w - bmp->w)/2;
    dstrect.y = (screen->h - bmp->h)/2;


    dstrect_SDL.x=dstrect.x;
    dstrect_SDL.y=dstrect.y;
    SDL_BlitSurface(bmp, 0, screen, &dstrect_SDL);
    SDL_Flip(screen);

    /* point de départ : le milieu de l'écran */
    dstrect.x = screen->w / 2;
    dstrect.y = screen->h /2;

    //sleep(1);
    return 0;
}

void down(void)
{
        crayon=1;
}

void goto_xy(signed short x, signed short y)
{
    coordonnees point;

    point.x = x + screen->w / 2;
    point.y = screen->h / 2 - y;

    if (crayon==1)
    {
        ligne(screen,dstrect,point);
    }
    else
    {

    }

    dstrect=point;
}

void ligne(SDL_Surface* surf,coordonnees A,coordonnees B)
{
    signed short pas,i,ax,ay,bx,by,i_fin;

    ax=A.x;
    ay=A.y;
    by=B.y;
    bx=B.x;

    if (ax==bx)
    {
        i=_min(ay,by);
        by=max(ay,by);
        while(i<=by)
        {
            EcrireCouleur(surf,ax,i);
            i++;
        }
    }
    else if (ay==by)
    {
        i=_min(ax,bx);
        bx=max(ax,bx);
        while(i<=bx)
        {
            EcrireCouleur(surf,i,by);
            i++;
        }
    }
    else if (abs(ax-bx)>abs(ay-by))
    {
        if (ax>bx)
        {
            pas=-1;
        }
        else
        {
               pas=1;
        }
        i_fin=bx-ax;
        for(i=0;i!=i_fin;i+=pas)
        {
            EcrireCouleur(surf,ax+i,ay+((by-ay)*i)/(bx-ax));
        }
        EcrireCouleur(surf,ax+i,ay+((by-ay)*i)/(bx-ax));
    }
    else
    {
        if (ay>by)
        {
            pas=-1;
        }
        else
        {
            pas=1;
        }
        i_fin=by-ay;
        for(i=0;i!=i_fin;i+=pas)
        {
            EcrireCouleur(surf,ax+((bx-ax)*i)/(by-ay),ay+i);
        }
        EcrireCouleur(surf,ax+((bx-ax)*i)/(by-ay),ay+i);
    }
    SDL_Flip(surf);
}

void SDL_EcrireCouleurAux(SDL_Surface* surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;


  if (x>=0 && x<=screen->w && y>=0 && y<=screen->h)
  {
      switch(bpp)
      {
          case 1:
          *p = pixel;
          break;

          case 2:
          *(Uint16 *)p = pixel;
          break;

          case 3:
          if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
          {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
          }
          else
          {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
          }
          break;

          case 4:
          *(Uint32 *)p = pixel;
          break;
      }
  }
  else
  {
    printf("Sort de la plage : x= %d et y=%d\n",x,y);
  }
}

void EcrireCouleur(SDL_Surface* surface, int x, int y)
{
    if (rvb==NULL)
    {
        SDL_EcrireCouleurAux(surface,x,y,~SDL_LireCouleurAux(surface,x,y));
    }
    else
    {
        SDL_EcrireCouleur(surface,x,y,rvb->rouge,rvb->vert,rvb->bleu);
    }
}

unsigned long SDL_LireCouleurAux(const SDL_Surface* surface, int x, int y)
{
  int bpp = surface->format->BytesPerPixel;

  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  if (x>=0 && x<=screen->w && y>=0 && y<=screen->h)
  {
      switch(bpp)
      {
          case 1:
          return *p;
          case 2:
          return *(Uint16 *)p;
          case 3:
          if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
          else
            return p[0] | p[1] << 8 | p[2] << 16;
          case 4:
          return *(Uint32 *)p;
          default:
          return 0;
      }
  }
  else
  {
    printf("Sort de la plage : x= %d et y=%d\n",x,y);
  }
  return 0;
}

static void SDL_LireCouleur(const SDL_Surface * surface, int x, int y, Uint8 * r, Uint8 * g, Uint8 * b)
{

  SDL_GetRGB(SDL_LireCouleurAux(surface, x, y), surface->format,
             (Uint8*)   r, (Uint8*) g, (Uint8*) b);

}

void SDL_EcrireCouleur(SDL_Surface* surface, int x, int y, int r, int g, int b)
{
  if (x>=0 && x<=screen->w && y>=0 && y<=screen->h)
  {
      SDL_EcrireCouleurAux(surface, x, y,
                           SDL_MapRGB(surface->format, r, g, b));
  }
  else
  {
    printf("Sort de la plage : x= %d et y=%d\n",x,y);
  }

}
void up(void)
{
    crayon=0;
}
void croix(void)
{
        down();
        goto_xy(x+100,y+100);
        up();
        goto_xy(x+100,y);
        down();
        goto_xy(x,y+100);
        up();
        sleep(0.1);

}

void disque()
{
    unsigned short dx,dy,r2;
    signed short i;

    up();
    goto_xy(x+50,y+50);
    down();
    r2=rayon*rayon;
    dx=rayon;
    for (i=-dx;i<=dx;i++)
    {
        EcrireCouleur(screen,dstrect.x+i,dstrect.y);
    }
    for (dy=1; dy<=rayon;dy++)
    {
        for (i=-dx;i<=dx;i++)
        {
            EcrireCouleur(screen,dstrect.x+i,dstrect.y+dy);
            EcrireCouleur(screen,dstrect.x+i,dstrect.y-dy);
        }
        while ((dx*dx+dy*dy)>=r2)
        {
            dx--;
        }
        dx++;
    }
    SDL_Flip(screen);
}

void gpio_inito(void)
{
    gpio_touchH = gpio_create(GPIO_HAUT, DIR_IN);
    gpio_touchB= gpio_create(GPIO_BAS, DIR_IN);
    gpio_touchD = gpio_create(GPIO_DROITE, DIR_IN);
    gpio_touchG= gpio_create(GPIO_GAUCHE, DIR_IN);
    gpio_touch = gpio_create(17, DIR_IN);
}

void gauche_100(void) //ok
{
    up();
    goto_xy(x-100,y);
    down();
    x=x-100;
    goto_xy(x,y);
    disque();
    sleep(0.4);
}

void droite_100(void) //ok
{
    up();
    goto_xy(x+100,y);
    down();
    x=x+100;
    goto_xy(x,y);
    sleep(0.4);
}

void bas_100(void) //ok
{
    up();
    goto_xy(x,y-100);
    down();
    goto_xy(x,y-100);
    y=y-100;
    usleep(750);
}

void haut_100(void) //ok
{
    up();
    goto_xy(x,y+100);
    down();
    goto_xy(x,y+100);
    y=y+100;
    usleep(750);
}

/************************************************************************************************************/
/************************************************************************************************************/
