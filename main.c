#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include "main.h"
//81m thusfar

int initialise() {
	SDL_Init(SDL_INIT_VIDEO);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("graphics/bglayer.png");
	fgLayer=IMG_Load("graphics/fglayer.png");
	return 0;
}

void loop() {
	static SDL_Rect bgScroll={0,0,WIDTH,HEIGHT};
	static SDL_Rect fgScroll={0,0,WIDTH,HEIGHT};
	if(bgScroll.x<WIDTH) bgScroll.x++;
	else bgScroll.x=0;
	if(fgScroll.x<WIDTH) fgScroll.x+=2;
	else fgScroll.x=0;
	SDL_Rect bgScroll2=bgScroll;
	SDL_Rect fgScroll2=fgScroll;
	bgScroll2.x-=WIDTH;
	fgScroll2.x-=WIDTH;
	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll);
	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll2);
	SDL_BlitSurface(fgLayer,NULL,s,&fgScroll);
	SDL_BlitSurface(fgLayer,NULL,s,&fgScroll2);
	SDL_UpdateWindowSurface(w);
}

int main() {
	if(initialise()) {
		printf("Initialisation failed.\n");
		return 1;
	}
	emscripten_set_main_loop(loop,30,1);
	return 0;
}