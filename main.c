#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <stdio.h>
//131m thusfar

#define TITLE "Diverse Abilities Sensory Screensaver"
#define WIDTH 640
#define HEIGHT 512
#define SIZE 64 //The size of each sprite.
#define ROWS 6 //The number of 64 pixel tall rows on the screen.
#define FISHCOUNT 7 //The number of fish sprites on the sheet.

SDL_Window* w;
SDL_Surface* s;
SDL_Surface* bgLayer;
SDL_Surface* fgLayer;
SDL_Surface* fishSprites;

typedef struct fish { //The basic fish object.
	short alive; //Non-zero if the fish is alive.
	int x; //Their X Position.
	int speed; //Their speed.
	unsigned int type; //Their sprite, counted from the top of the sheet down.
} fish;

fish fishList[ROWS];//Creates a fish object for each row on the screen.

int initialise() {
	memset(&fishList,0,sizeof fishList);//Initialises the fish array.
	SDL_Init(SDL_INIT_VIDEO);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("graphics/bglayer.png");
	fgLayer=IMG_Load("graphics/fglayer.png");
	fishSprites=IMG_Load("graphics/fish.png");//All of the fish are stored on one sheet and are separated by the drawFish function.
	return 0;
}

void fishTick() {
	for(int i=0;i<ROWS;i++){
		printf("I: %d\n",i);
		if(fishList[i].alive) {
			if(fishList[i].x<WIDTH) fishList[i].x+=fishList[i].speed;//If the fish is alive, and on the screen, move it.
			else fishList[i].alive=0;//If it's off the screen, kill it.
			SDL_Rect fishPosition={fishList[i].x,i*SIZE+SIZE,SIZE,SIZE};//The fish's Y co-ordinate is determined by its position in the array.
			SDL_Rect spriteOnSheet={0,fishList[i].type*SIZE,SIZE,SIZE};//The sprite's position and dimensions on the spritesheet.
			SDL_BlitSurface(fishSprites,&spriteOnSheet,s,&fishPosition);
		} else {
			if(rand()%2){
				fishList[i].type=rand()%(FISHCOUNT-1);//Selects a random fish from the sheet.
				fishList[i].speed=rand()%3+5;//Sets the speed to a random number between five and eight pixels per frame.
				fishList[i].alive=1;
				fishList[i].x=-64;//Sets it slightly offscreen so that the player won't perceive pop-in.
			}
		}
	}
}

void loop() {
	static SDL_Rect bgScroll={0,0,WIDTH,HEIGHT};//Background layer position.
	static SDL_Rect fgScroll={0,0,WIDTH,HEIGHT};//Foreground layer position.

	if(bgScroll.x<WIDTH) bgScroll.x++;
	else bgScroll.x=0;

	if(fgScroll.x<WIDTH) fgScroll.x+=2;//The foreground is moved by two pixels per frame rather than one to simulate parallax.
	else fgScroll.x=0;

	SDL_Rect bgScroll2=bgScroll;//Creates second set of variables containing layer positions.
	SDL_Rect fgScroll2=fgScroll;
	bgScroll2.x-=WIDTH;//Offsets them left by a screen width from the main set.
	fgScroll2.x-=WIDTH;

	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll);//Draws the background layer in two positions to produce an illusion of a seamless backgroung.
	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll2);

	fishTick();

	SDL_BlitSurface(fgLayer,NULL,s,&fgScroll);//The same, only for the foreground.
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