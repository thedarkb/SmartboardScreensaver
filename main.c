#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
//220m thusfar

#define TITLE "Diverse Abilities Sensory Screensaver"
#define WIDTH 640
#define HEIGHT 512
#define SIZE 64 //The size of each sprite.
#define ROWS 6 //The number of 64 pixel tall rows on the screen.
#define FISHCOUNT 7 //The number of fish sprites on the sheet.

SDL_Window* w;
TTF_Font* f;

SDL_Surface* s;
SDL_Surface* bgLayer;
SDL_Surface* fgLayer;
SDL_Surface* fishSprites;
SDL_Surface* scoreLabel;
SDL_Surface* quarryLabel;

typedef struct fish { //The basic fish object.
	short alive; //Non-zero if the fish is alive.
	int x; //Their X Position.
	int speed; //Their speed.
	unsigned int type; //Their sprite, counted from the top of the sheet down.
} fish; //Each fish occupies its own row, of height SIZE, on the screen. 

fish fishList[ROWS];//Creates a fish object for each row on the screen.

int score=0;
int highScore=100;
unsigned char quarry=0;//Contains the target fish type.

int initialise() {
	memset(&fishList,0,sizeof fishList);//Initialises the fish array.
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	f=TTF_OpenFont("graphics/font.ttf",40);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("graphics/bglayer.png");
	fgLayer=IMG_Load("graphics/fglayer.png");
	fishSprites=IMG_Load("graphics/fish.png");//All of the fish are stored on one sheet and are separated by the drawFish function.
	quarryLabel=IMG_Load("graphics/qlabel.png");
	quarry=rand()%(FISHCOUNT-1);
	printf("Initial Quarry: %d\n", quarry);
	return 0;
}

void fishTick() {
	for(int i=0;i<ROWS;i++){
		if(fishList[i].alive) {
			if(fishList[i].x<WIDTH) fishList[i].x+=fishList[i].speed;//If the fish is alive, and on the screen, move it.
			else fishList[i].alive=0;//If it's off the screen, kill it.
			SDL_Rect fishPosition={fishList[i].x,i*SIZE+SIZE,SIZE,SIZE};//The fish's Y co-ordinate is determined by its position in the array.
			SDL_Rect spriteOnSheet={0,fishList[i].type*SIZE,SIZE,SIZE};//The sprite's position and dimensions on the spritesheet.
			SDL_BlitSurface(fishSprites,&spriteOnSheet,s,&fishPosition);//Blits sprite to the screen.
		} else {
			if(rand()%2){
				fishList[i].type=rand()%(FISHCOUNT-1);//Selects a random fish from the sheet.
				fishList[i].speed=(rand()%3)+5;//Sets the speed to a random number between five and eight pixels per frame.
				fishList[i].alive=1;
				fishList[i].x=-64;//Sets it slightly offscreen so that the player won't perceive pop-in.
			}
		}
	}
}

void hitCheck() {
	int checkX;
	int checkY;
	static int lastClick=0;
	if(!lastClick && SDL_GetMouseState(&checkX,&checkY) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		//If the mouse was not pressed on the previous frame, and the left button is pressed.
		if(checkX > fishList[(checkY/SIZE)-1].x && checkX < fishList[(checkY/SIZE)-1].x+SIZE && checkY<HEIGHT-SIZE && checkY>SIZE) {
			/*If the mouse cursor Y position is greater than SIZE and less than HEIGHT-SIZE; and checkX is within 64 pixels 
			of the X position of the fish on that row.*/
			if(fishList[(checkY/SIZE)-1].type==quarry) {
				score+=500;
				quarry=rand()%(FISHCOUNT-1);//Select a new fish type to pursue.
				fishList[(checkY/SIZE)-1].alive=0;
				printf("Fish hit registered.\n");
			}
		}
	}

	if(!SDL_GetMouseState(NULL,NULL)) lastClick=0;
	else lastClick=1;
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

	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll);//Draws the background layer in two positions to produce an illusion of a seamless background.
	SDL_BlitSurface(bgLayer,NULL,s,&bgScroll2);

	fishTick();
	hitCheck();

	SDL_BlitSurface(fgLayer,NULL,s,&fgScroll);//The same, only for the foreground.
	SDL_BlitSurface(fgLayer,NULL,s,&fgScroll2);

	SDL_Rect qlabelRect={WIDTH-SIZE*3,HEIGHT-SIZE,0,0};

	SDL_BlitSurface(quarryLabel,NULL,s,&qlabelRect); //Draws the quarry's HUD graphic.
	qlabelRect.x+=SIZE*2;
	SDL_Rect spriteOnSheet={0,quarry*SIZE,SIZE,SIZE};//The sprite's position and dimensions on the spritesheet.
	SDL_BlitSurface(fishSprites,&spriteOnSheet,s,&qlabelRect);

	SDL_Color fC={255,255,255};//Pure white text.
	char* scoreMsg;
	sprintf(scoreMsg,"Score: %d", score); //Creates a string to display the current score.
	scoreLabel=TTF_RenderText_Solid(f,scoreMsg,fC);//Renders the message using font 'f', message 'scoreMsg', and colour 'fC'.
	SDL_BlitSurface(scoreLabel,NULL,s,NULL);//Blits the rendered message to the screen.

	SDL_UpdateWindowSurface(w);
	if(score>highScore) highScore=score;
	if(score>0) score--;
}

int main() {
	if(initialise()) {
		printf("Initialisation failed.\n");
		return 1;
	}
	emscripten_set_main_loop(loop,30,1);
	return 0;
}