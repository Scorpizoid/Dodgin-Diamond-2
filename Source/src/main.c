/*

    Dodgin' Diamond 2, a shot'em up arcade
    Copyright (C) 2003,2004 Juan J. Martinez <jjm@usebox.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include<stdlib.h>
#include<stdio.h>
#include"main.h"
#include"SDL.h"
#if !AUDIO_DISABLED
	#include"SDL_mixer.h"
#endif
#include"menu.h"

#define APP_NAME	"Dodgin' Diamond ]["

#define FPS	60

#include "control.h"
#include "SDL_plus.h"
#include "engine.h"
#include "cfg.h"

#ifdef WIN32
static const char COPYRIGHT[]="Dodgin' Diamond 2 - Copyright (c) 2003,2004 Juan J. Martinez <jjm@usebox.net> This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License; either version 2 of the License, or (at your option) any later version, as published by the Free Software Foundation (www.fsf.org).";
#endif

SDL_Surface *screen, *gfx;
extern pDesc player[2];
#if !JOYSTICK_DISABLED
SDL_Joystick *joy[2]={ NULL, NULL };
#endif
SDL_Event event;
Uint32 tick, ntick;
float scroll=0,scroll2=0;

bool pause;
Uint32 pause_tick;

extern bool boss;

cfg conf;
score hiscore[10];

#if !AUDIO_DISABLED
Mix_Chunk *efx[8]={ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
Mix_Music *bgm=NULL, *bgm_boss=NULL;
int sound;
#endif
bool done;

/* load all the sound stuff */
/*void
soundLoad()
{
    int i;
    char buffer[512];

	sprintf(buffer,"c:\\resource\\apps\\dd2\\bgm1.xm");
	bgm=Mix_LoadMUS(buffer);
	if(!bgm)
		fprintf(stderr,"Unable load bgm: %s\n", SDL_GetError());

	sprintf(buffer,"c:\\resource\\apps\\dd2\\bgm2.xm");
	bgm_boss=Mix_LoadMUS(buffer);
	if(!bgm_boss)
		fprintf(stderr,"Unable load bgm_boss: %s\n", SDL_GetError());

	for(i=0;i<NUM_EFX;i++) {
		sprintf(buffer,"c:\\resource\\apps\\dd2\\efx%i.wav",i+1);
		efx[i]=Mix_LoadWAV(buffer);
		if(!efx[i]) {
			fprintf(stderr,"Unable load efx: %s\n", SDL_GetError());
		} else
			Mix_VolumeChunk(efx[i],MIX_MAX_VOLUME/2);
	}

}
*/
void
gameLoop()
{
	int afterdeath=0;

	for(done=false,tick=SDL_GetTicks();!done && afterdeath<400;) {

		while(SDL_PollEvent(&event)) {
			if (event.type==SDL_QUIT)
				done=true;

			/* joystick control for the menu */
		#if !JOYSTICK_DISABLED
			if(player[0].joy && joy[0])
			{
				SDL_JoystickUpdate();

				if(SDL_JoystickGetButton(joy[0], 1))
				{
					event.type=SDL_KEYDOWN;
					event.key.keysym.sym=SDLK_p;
				}
			}
#if !PLAYER2_DISABLED
			else
				if(player[1].joy && joy[1])
				{
					SDL_JoystickUpdate();

					if(SDL_JoystickGetButton(joy[1], 1))
					{
						event.type=SDL_KEYDOWN;
						event.key.keysym.sym=SDLK_p;
					}
				}
#endif
		#endif
			if(event.type==SDL_KEYDOWN) {
				if(event.key.keysym.sym==SDLK_ESCAPE) {
					done=true;
					continue;
				}
				else
				{
					if(event.key.keysym.sym==SDLK_p && pause_tick<SDL_GetTicks())
					{
						writeCString(gfx, screen, 98, 20, "game paused", 0);
						SDL_Flip(screen);
						pause=pause ? false : true;
						pause_tick=SDL_GetTicks()+200;
						continue;
					}
					else
						if(event.key.keysym.sym==SDLK_F12)
							SDL_SaveBMP(screen,"scnshot.bmp");
				}
			}
		}

		/* player control */
		if(player[0].shield) {
#if !JOYSTICK_DISABLED
			if(joy[0] && player[0].joy)
				control_player_joy(joy[0],&player[0]);
			else
#endif
				control_player(&player[0]);
		}

#if !PLAYER2_DISABLED && !JOYSTICK_DISABLED
		if(player[1].shield) {
			if(joy[1] && player[1].joy)
				control_player_joy(joy[1],&player[1]);
			else
				control_player(&player[1]);
		}
#endif

		if(pause)
			continue;
		
		/* frame rate calculation */
		ntick=SDL_GetTicks();
		if(ntick-tick>=1000/FPS) {
			tick=ntick;

				/* scroll background here */
			{
				SDL_Rect a,b;

				if(scroll>0)
					scroll-=0.5;
				else
					scroll=200;

				b.x=1;
				b.w=SCREENW;
				a.x=0+160; //MUUTOS: 0

				if(!scroll) {
					a.y=0;
					b.y=204;
					b.h=SCREENH;
					SDL_BlitSurface(gfx, &b, screen, &a);
				} else {
					a.y=0;
					b.y=204+(int)scroll;
					b.h=SCREENH-(int)scroll;
					SDL_BlitSurface(gfx, &b, screen, &a);
					a.y=SCREENH-(int)scroll;
					b.y=204;
					b.h=(int)scroll;
					SDL_BlitSurface(gfx, &b, screen, &a);
				}

// The side images

				//The left one
				/* a.x = the x value on the screen where the image starts showing
				   a.y = the y value on the screen
				   b.x = the x value where the image gets loaded in gfx.bmp
				   b.y = the y value on the gfx.bmp
				*/
				b.x=466;
				b.w=160;
				a.x=0;

				a.y=0;
				b.y=388;
				b.h=200;
				SDL_BlitSurface(gfx, &b, screen, &a);

				//And the right one
				b.x=630;
				b.w=160;
				a.x=480;

				a.y=0;
				b.y=388;
				b.h=200;
				SDL_BlitSurface(gfx, &b, screen, &a);


				/* scroll parallax here */

				if(scroll2>0)
					scroll2-=2;
				else
					scroll2=200;

				b.x=324;
				b.w=24;
				a.x=0+160;

				if(!scroll2) {
					a.y=0;
					b.y=204;
					b.h=SCREENH;
					SDL_BlitSurface(gfx, &b, screen, &a);

					b.x=359;
					a.x=SCREENW-24+160;
					SDL_BlitSurface(gfx, &b, screen, &a);		  		
				} else {
					a.y=0;
					b.y=204+(int)scroll2;
					b.h=SCREENH-(int)scroll2;
					SDL_BlitSurface(gfx, &b, screen, &a);
					a.y=SCREENH-(int)scroll2;
					b.y=204;
					b.h=(int)scroll2;
					SDL_BlitSurface(gfx, &b, screen, &a);

					b.x=359;
					a.x=SCREENW-24+160;
					a.y=0;
					b.y=204+(int)scroll2;
					b.h=SCREENH-(int)scroll2;
					SDL_BlitSurface(gfx, &b, screen, &a);
					a.y=SCREENH-(int)scroll2;
					b.y=204;
					b.h=(int)scroll2;
					SDL_BlitSurface(gfx, &b, screen, &a);	
				}
			}
			/* enemy here */
			engine_enemy();

			/* fire here */
			engine_fire();

			/* character here */
			if(player[0].shield)
				engine_player(&player[0]);

#if !PLAYER2_DISABLED
			if(player[1].shield)
				engine_player(&player[1]);
#endif

			if(!(player[0].shield/* | player[1].shield*/))
				afterdeath++;
				
			engine_obj();

			engine_vefx();

			/* panel */
			drawPanel(gfx,screen,player);

			SDL_Flip(screen);
		}
	}
}

int
main (int argc, char *argv[])
{
    int i,j,k;
    char buffer[512];
	SDL_VideoInfo *vi;
	unsigned char bpp=8;

#ifndef WIN32
	if(argc==2)
		if(argv[1][0]=='-' && argv[1][1]=='v') {
			printf("dd2 for S80 v1.01\nCopyright (c) 2003,2004 Juan J. Martinez <jjm@usebox.net>\n");
			printf("This is free software, and you are welcome\nto redistribute it"
                   " under certain conditions; read COPYING for details.\n");
			return 1;
		}

    /* try local configuration */
	sprintf(buffer,"%.500s/.dd2rc",getenv("HOME"));
	if(!loadCFG(buffer,&conf)) {
		/* if there's no local, use global */
		sprintf(buffer,"Data\\dd2.cfg");
		if(!loadCFG(buffer,&conf)) {
			fprintf(stderr,"unable to read configuration, using defaults\n");
		}
	}
#else
	sprintf(buffer,"Data\\dd2.cfg");
	if(!loadCFG(buffer,&conf)) {
		fprintf(stderr,"unable to read configuration, using defaults\n");
	}
#endif

	/* read hi-scores */
	sprintf(buffer,"Data\\dd2-hiscore");
	if(!loadScore(buffer,hiscore)) {
		fprintf(stderr,"unable to read hi-scores, using defaults\n");
	}

	/* init only video */
	i=SDL_Init(SDL_INIT_VIDEO);
	if(i<0) {
		fprintf(stderr,"Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

#if !AUDIO_DISABLED
	sound=SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO;
#endif

	/* no sound, 16000, 22050, 44100 */
#if !AUDIO_DISABLED
	if(sound && conf.sound!=NO_SOUND) {
		switch(conf.sound) {
			default:
			case SOUND_HI:
				i=44100;
			break;
			case SOUND_MED:
				i=22050;
			break;
			case SOUND_LOW:
				i=16000;
			break;
		}
#endif
#if !AUDIO_DISABLED
		if(Mix_OpenAudio(i, MIX_DEFAULT_FORMAT, 2, 2048)<0) {
			fprintf(stderr, "Unable to set audio: %s\n", SDL_GetError());
			sound=0;
		} else
			soundLoad();
	}
#endif

	vi=(SDL_VideoInfo *)SDL_GetVideoInfo();
	if(vi)
		bpp=vi->vfmt->BitsPerPixel;

	if(conf.fullscreen)
		screen=SDL_SetVideoMode(640, SCREENH, bpp, SDL_FULLSCREEN);
	else
		screen=SDL_SetVideoMode(640, SCREENH, bpp, SDL_SWSURFACE);

	if(!screen) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}

#if !JOYSTICK_DISABLED
	/* init the joystick */
	if(SDL_WasInit(SDL_INIT_JOYSTICK) & SDL_INIT_JOYSTICK)
		if(SDL_NumJoysticks()>=1)
		{
			joy[0]=SDL_JoystickOpen(0);
			if(SDL_NumJoysticks()>1)
				joy[1]=SDL_JoystickOpen(1);
		}
#endif

	/* hide the mouse */
	SDL_ShowCursor(SDL_DISABLE);

	/* set the caption */
	SDL_WM_SetCaption(APP_NAME,NULL);

	/* load console gfx */
	sprintf(buffer,"Data\\gfx.bmp");
	gfx=loadBMP(buffer);
	if(!gfx) {
		fprintf(stderr,"Unable load gfx: %s\n", SDL_GetError());
		return 1;
	}
	/* set transparent color */
	if(SDL_SetColorKey(gfx, SDL_SRCCOLORKEY, SDL_MapRGB(gfx->format, 255, 0, 255))<0) {
			fprintf(stderr,"Unable to setup gfx: %s\n", SDL_GetError());
			return 1;
	}

	/* main LOOP */
	while(menu()) {

		/* init the engine */
		engine_init();

		SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,0,0,0));
		SDL_Flip(screen);

#if !AUDIO_DISABLED
		if(sound && bgm) {
			Mix_VolumeMusic(MIX_MAX_VOLUME);
			Mix_FadeInMusic(bgm,-1,2000);
			SDL_Delay(2000);
#endif		}

#if !JOYSTICK_DISABLED && PLAYER2_DISABLED
		player[0].joy=(int)conf.control[0]==JOYSTICK;
		player[1].joy=(int)conf.control[1]==JOYSTICK;
#endif
		pause=0;
		pause_tick=0;
		boss=0;
		gameLoop();

#if !AUDIO_DISABLED
		if(sound && bgm) {
			Mix_FadeOutMusic(2000);
			SDL_Delay(3000);
		}
#endif

		for(i=0;i<1;i++) {
			/* check if there's a place for this score */
			for(j=9;j>=0 && hiscore[j].score<player[i].score;j--);

			/* the player will be in the hall of fame? */
			if(j<9) {
				for(k=8;k>j;k--)
					hiscore[k+1]=hiscore[k];

				/* put the new score */
				hiscore[j+1].score=player[i].score;
				hiscore[j+1].stage=player[i].stage;

				hiscore[j+1].name[0]=0;
				if(!getName(hiscore[j+1].name, j+2,i+1))
					break; /* probably a problem if the user closes the window */

				/* show the hall of fame */
				hiscores();
			}
		}
	}

#if !AUDIO_DISABLED
	if(sound) {
		if(bgm)
			Mix_FreeMusic(bgm);
		if(bgm_boss)
			Mix_FreeMusic(bgm_boss);

		for(i=0;i<NUM_EFX;i++)
			if(efx[i])
				Mix_FreeChunk(efx[i]);

		Mix_CloseAudio();
	}
#endif

#if !JOYSTICK_DISABLED
	/* release the joystick */
	if(SDL_JoystickOpened(0))
		SDL_JoystickClose(joy[0]);
	if(SDL_JoystickOpened(1))
		SDL_JoystickClose(joy[1]);
#endif

	/* free all! */
	SDL_FreeSurface(gfx);

	/* free engine memory */
	engine_release();

	/* now update conf changes */
	sprintf(buffer,"Data\\dd2.cfg");
	if(!saveCFG(buffer,&conf)) {
		fprintf(stderr,"unable to save hi-scores\ndo you have permissions to write into %s?\n"
			,buffer);
	}

		
	/* save hi-scores */
	sprintf(buffer,"Data\\dd2-hiscore");
	saveCFG(buffer,&conf);
	if(!saveScore(buffer,hiscore)) {
		fprintf(stderr,"unable to save hi-scores\ndo you have permissions to write into %s?\n"
			,buffer);
	}

	return 0;
}
