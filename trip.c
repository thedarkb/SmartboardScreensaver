#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

#define TITLE "Platformer"
#define WIDTH 640
#define HEIGHT 512
#define SIZE 64
//115m

SDL_Window* w;
TTF_Font* f;
SDL_Surface* s;
SDL_Surface* bgLayer;
SDL_Surface* tripMan;
SDL_Surface* scoreLabel;

void blit(SDL_Surface* in, int x, int y, int w, int h) {//For background layers and things.
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(in,NULL,s,&dest);
}

void blitSprite(SDL_Surface* sprite, unsigned char index, int x, int y, int w, int h) {//For things from the spritesheet.
	SDL_Rect source={0,index*SIZE,SIZE,SIZE};
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(sprite,&source,s,&dest);
}

void loop() {
	static int box=0;//The X position of the blob monster.
	static int bgScroll=WIDTH;//X position of the parallax background layer.
	static int jump=0;//Player's Y position.
	static unsigned int frameCounter=0;//Number of frames elapsed since the start of the game.
	static int changeRate=60;//Tracks the player's position in their jump.
	static short direction=0;//non-zero if the player is jumping.
	static int released=0;//Non-zero if a mouse button was pressed on the previous frame.
	static int speed=10;//The speed at which the blob monsters advance.
	static unsigned int score=0;
	static unsigned int highScore=0;
	frameCounter++;
	score++;

	if(!SDL_GetMouseState(NULL,NULL)) released=0;
	
	if(!direction && SDL_GetMouseState(NULL,NULL) & SDL_BUTTON(SDL_BUTTON_LEFT) && !released) direction=1;
	if(direction||jump) {//Jump physics algorithm.
		jump+=changeRate;
		changeRate-=10;
		if(jump<=0) {
			direction=0;
			jump=0;
			changeRate=60;
		}
		released=1;
	}

	if(bgScroll>0) bgScroll--;//IF-ELSE to present the illusion of a seamless background.
	else bgScroll=WIDTH;

	blit(bgLayer,bgScroll,0,WIDTH,HEIGHT);//Draws the background image.
	blit(bgLayer,bgScroll-WIDTH,0,WIDTH,HEIGHT);//Draws the background image again, offset by a screen's width.
	blitSprite(tripMan,(frameCounter/4)%2,SIZE,377-jump,SIZE,SIZE);//Draws the player's sprite.

	if(box<-SIZE) box=WIDTH*((rand()%2)+1);//If the box is more than 64 pixels off the screen, respawn the box at a random distance to the right.
	else box-=speed;//Moves the box normally.
	blitSprite(tripMan,2,box,377,SIZE,SIZE);//Draws the box.

	if(!(frameCounter%20)) speed++;//Increase the speed by one pixel per frame every twenty frames.

	if(box<2*SIZE && box>SIZE && jump<SIZE-10) {//If the player collides with the blob.
		score=0;//Reset their score.
		speed=10;//Reset the speed.
	}
	if(score>highScore) highScore=score;

	SDL_Color fC={255,255,255};//Pure white text.
	char* scoreMsg="The quick brown fox jumped over the lazy dog.";
	sprintf(scoreMsg,"Score: %d", score); //Creates a string to display the current score.
	scoreLabel=TTF_RenderText_Solid(f,scoreMsg,fC);//Renders the message using font 'f', message 'scoreMsg', and colour 'fC'.
	SDL_BlitSurface(scoreLabel,NULL,s,NULL);//Blits the rendered message to the screen.

	sprintf(scoreMsg,"High Score: %d", highScore); //Creates a string to display the current high score.
	scoreLabel=TTF_RenderText_Solid(f,scoreMsg,fC);//Renders the message using font 'f', message 'scoreMsg', and colour 'fC'.
	SDL_Rect hsOffset={4,HEIGHT-SIZE,0,0};//Positions the High Score display at the bottom of the screen.
	SDL_BlitSurface(scoreLabel,NULL,s,&hsOffset);//Blits the rendered message to the screen.

	SDL_UpdateWindowSurface(w);
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	f=TTF_OpenFont("tripgraph/font.ttf",40);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("tripgraph/bg.png");
	tripMan=IMG_Load("tripgraph/tripman.png");
	printf("Game initialised.\n");
	emscripten_set_main_loop(loop,30,1);
	return 0;
}