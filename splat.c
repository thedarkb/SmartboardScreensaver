#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
//221 minutes thusfar.

#define TITLE "Splat the Alien"
#define WIDTH 640
#define HEIGHT 512
#define SIZE 64

#define ALIENLIMIT 10

SDL_Window* w;
SDL_Surface* s;
TTF_Font* f;

SDL_Surface* bgLayer;
SDL_Surface* fgLayer;
SDL_Surface* spriteSheet;

unsigned int frameCounter=0;
unsigned int highScore=0;
unsigned int score=0;

struct alien {//The main alien object.
	char alive;//Non-zero if the alien is alive.
	int y;//Alien's Y position.
	int type;//Its sprite.
	int speed;//Its speed.
	int fx;//The rate at which its speed changes.
} aliens[ALIENLIMIT];//Array of aliens, one for each column of width SIZE on the screen.

struct projectile {//Projectile object.
	char alive;//Non-zero if the projectile is in flight.
	int x;//X position.
	int y;//Y position.
	int quarry;//The alien it's locked on to.
} projectile;

struct particle {//Particle object.
	int y;//Y Position.
	int type;//Sprite
} particles[ALIENLIMIT];//Array of particles, one for each column of width SIZE on the screen.

void blit(SDL_Surface* in, int x, int y, int w, int h) {//For background layers and things.
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(in,NULL,s,&dest);
}

void blitSprite(SDL_Surface* sprite, unsigned char index, int x, int y, int w, int h) {//For things from the spritesheet.
	SDL_Rect source={0,index*SIZE,SIZE,SIZE};
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(sprite,&source,s,&dest);
}

void enemyTick() {
	for(int i=0;i<ALIENLIMIT;i++) {//Steps through the alien array.
		if(aliens[i].alive) {
			if(!(frameCounter%12)) aliens[i].speed-=aliens[i].fx;//Change the alien's speed every twelve frames by fx.
			aliens[i].y+=aliens[i].speed;//Shifts the alien on the Y axis.
			if(aliens[i].y>HEIGHT || aliens[i].y<-SIZE*2) {
				aliens[i].alive=0;//Kills the alien if it leaves the screen.
				if(score>0) score-=100;
			}
			else blitSprite(spriteSheet,aliens[i].type,i*SIZE,aliens[i].y,SIZE,SIZE);//Draws the alien on the screen.
		} else if(!(rand()%3)) {//If the RNG outputs a multiple of three...
			printf("Spawning new enemy\n");
			aliens[i].y=-SIZE;//Teleports aliens[i] above the top of the screen.
			aliens[i].alive=1;//Sets its state to "alive".
			aliens[i].type=(rand()%5);//Changes its sprite.
			aliens[i].speed=(rand()%5)+5;//Changes its default speed.
			aliens[i].fx=(rand()%3);//Changes the rate at which its speed changes.
		}
	}
}

void clickTick() {
	static char lastClick=0;//Non-zero if the mouse was pressed on the previous frame.
	int mX;
	int mY;

	if(!SDL_GetMouseState(NULL,NULL)) lastClick=0;
	if(SDL_GetMouseState(&mX,&mY) & SDL_BUTTON(SDL_BUTTON_LEFT) && !lastClick && !projectile.alive) {
		lastClick=1;
		if(aliens[mX/SIZE].alive && mY>aliens[mX/SIZE].y && mY<aliens[mX/SIZE].y+SIZE) {//If a living alien has been clicked.
			projectile.alive=1;//Spawns the projectile.
			projectile.x=WIDTH/2-SIZE/2;//Moves its X position to the centre of the screen.
			projectile.y=HEIGHT-SIZE*2;//Moves its Y position to 64 pixels above the player's spacecraft.
			projectile.quarry=mX/SIZE;//Locks the projectile to the clicked enemy.
		}
	}
}

void bulletTick() {
	if(!projectile.alive) return;
	blitSprite(spriteSheet,6,projectile.x,projectile.y,SIZE,SIZE);//Draws the projectile sprite.
	if(projectile.quarry*SIZE<projectile.x) projectile.x-=25;//If the target is left of the projectile, move the projectile left.
	if(projectile.quarry*SIZE>projectile.x) projectile.x+=25;//If the target is right of the projectile, move the projectile right.
	if(aliens[projectile.quarry].y<projectile.y) projectile.y-=25;//If the target is above the projectile, move the projectile up.
	if(aliens[projectile.quarry].y>projectile.y) projectile.y+=25;//If the target is below the projectile, move the projectile down.
	if(projectile.x>WIDTH || projectile.x<-SIZE) projectile.alive=0;//If the projectile leaves the screen on the X axis, kill the projectile.
	if(projectile.y>HEIGHT || projectile.y<-SIZE) projectile.alive=0;//Ditto, but for the Y axis.
	if(projectile.x>projectile.quarry*SIZE && projectile.x<projectile.x*SIZE+SIZE){//If the projectile has the same X coordinate as its target.
		if(projectile.y>aliens[projectile.quarry].y && projectile.y<aliens[projectile.quarry].y+SIZE){//If the projectile has the same Y coordinate as its target.
			aliens[projectile.quarry].alive=0;//Kill the target.
			projectile.alive=0;//Destroy the projectile.
			particles[projectile.quarry].y=aliens[projectile.quarry].y;
			particles[projectile.quarry].type=7;
			score+=1000;
		}
	}
}

void loop() {
	static int bgScroll=0;//The position of the rear parallax layer.
	static int fgScroll=0;//The position of the front parallax layer.

	if(score>highScore)highScore=score;

	if(bgScroll+1<HEIGHT) bgScroll++;//Moves the background layer if its position is less than the height of the screen.
	else bgScroll=0;//Else, reset the position.
	if(fgScroll+5<HEIGHT) fgScroll+=5;//Moves the front parallax layer five times faster.
	else fgScroll=0;//Resets the position if it exceeds the height of the screen.

	blit(bgLayer,0,bgScroll,WIDTH,HEIGHT);//These blits draw the foreground and background layers.
	blit(bgLayer,0,bgScroll-HEIGHT,WIDTH,HEIGHT);
	blit(fgLayer,0,fgScroll,WIDTH,HEIGHT);
	blit(fgLayer,0,fgScroll-HEIGHT,WIDTH,HEIGHT);

	enemyTick();//Handles the enemy logic.
	clickTick();//Handles user input.
	bulletTick();//Handles projectile logic.
	for(int i=0;i<ALIENLIMIT;i++) blitSprite(spriteSheet,particles[i].type,i*SIZE,particles[i].y+=50,SIZE,SIZE);

	blitSprite(spriteSheet,5,WIDTH/2-SIZE/2,HEIGHT-SIZE,SIZE,SIZE);//Draws the player.

	SDL_Surface* scoreLabel;
	SDL_Color fC={255,255,255};//Pure white text.
	char* scoreMsg="The quick brown fox jumped over the lazy dog.";
	sprintf(scoreMsg,"Score: %d", score); //Creates a string to display the current score.
	scoreLabel=TTF_RenderText_Solid(f,scoreMsg,fC);//Renders the message using font 'f', message 'scoreMsg', and colour 'fC'.
	blit(scoreLabel,4,4,0,0);

	sprintf(scoreMsg,"High Score: %d", highScore); //Creates a string to display the current high score.
	scoreLabel=TTF_RenderText_Solid(f,scoreMsg,fC);//Renders the message using font 'f', message 'scoreMsg', and colour 'fC'.
	blit(scoreLabel,4,HEIGHT-SIZE,0,0);


	SDL_UpdateWindowSurface(w);
	frameCounter++;
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	f=TTF_OpenFont("splatgraph/font.ttf",30);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("splatgraph/bglayer.png");
	fgLayer=IMG_Load("splatgraph/fglayer.png");
	spriteSheet=IMG_Load("splatgraph/spritesheet.png");
	memset(&projectile,0,sizeof projectile);
	memset(&aliens,0,sizeof aliens);
	memset(&particles,255,sizeof particles);
	emscripten_set_main_loop(loop,30,1);
	return 0;
}