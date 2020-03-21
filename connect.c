#include <emscripten.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>

#define TITLE "Connect Penguin"
#define WIDTH 640
#define HEIGHT 512
#define SIZE 64
#define GRAVITY 1

SDL_Window* w;//The window in the webpage.
SDL_Surface* s;//The surface of the window.
TTF_Font* f;//The large font, used by the turn label.
TTF_Font* fs;//The small font, used by the victory message.

SDL_Surface* bgLayer;
SDL_Surface* fgLayer;
SDL_Surface* spriteSheet;
const SDL_Color fC={255,255,255};//Font color, might be worth changing to black.

char mode=0;//The game's state.
char turn=1;

struct {//The main game object.
	int y;//Used by the drop physics.
	char team;//The type of token.
	int momentum;//Also used by the drop physics.
} board[7][6];

void blit(SDL_Surface* in, int x, int y, int w, int h) {//For background layers and things.
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(in,NULL,s,&dest);
}

void blitSprite(SDL_Surface* sprite, unsigned char index, int x, int y, int w, int h) {//For things from the spritesheet.
	SDL_Rect source={0,index*SIZE,SIZE,SIZE};
	SDL_Rect dest={x,y,w,h};
	SDL_BlitSurface(sprite,&source,s,&dest);
}

void winLoop() {
	static int frameCounter=0;
	frameCounter++;
	if(SDL_GetMouseState(NULL,NULL) && frameCounter>60) {//Ensures that an errant victory tap does not clear the board.
		memset(&board,0,sizeof board);//Resets board.
		mode=0;//Resets mode.
	} else {//Turn is not reset as it means the loser is the first to move in the next round.
		if(turn==1) blit(TTF_RenderText_Solid(fs,"Grey is the winner, tap the screen to play again!",fC),2,HEIGHT-64,0,0);
		else blit(TTF_RenderText_Solid(fs,"Red is the winner, tap the screen to play again!",fC),2,HEIGHT-64,0,0);
	}
}

void spriteDrop(char root, char dest) {//Drop physics.
	int yaynay=0;//One if block is still in motion.
	for(int x=0;x<7;x++) {
		for(int y=0;y<6;y++) {
			if(board[x][y].y-board[x][y].momentum>=0) {//If the block will not hit the bottom on this frame.
				board[x][y].y-=board[x][y].momentum;//Lower the block according to its momentum.
				board[x][y].momentum+=GRAVITY;//Increase its momentum according to gravity.
				yaynay=1;//Signals that a block is still in motion.
			} else board[x][y].y=0;//Snaps the block to the grid if it has come to a halt.
		}
	}
	if(yaynay) mode=root;//The calling state.
	else mode=dest;//The destination state.
}

void checkWin(){//Detects victory states.
	for(int x=0;x<7;x++){
		for(int y=0;y<6;y++){
			for(int i=0;i<4;i++) {//Detects a horizontal line.
				if(x+i>6||x+i<0)break;//Breaks rather than checking out of bounds.
				if(!board[x][y].team) break;//Breaks if the space is empty.
				if(board[x][y].team!=board[x+i][y].team) break;//Breaks if the horizontal line is broken.
				else if(i==3 && board[x][y].team==board[x+i][y].team)goto win;//Identifies a victory if a line of four is detected.
			}
			for(int i=0;i<4;i++) {//Ditto above, but for downward sloping diagonal lines.
				if(x+i>6||x+i<0)break;
				if(y+i>5||y+i<0)break;
				if(!board[x][y].team) break;
				if(board[x][y].team!=board[x+i][y+i].team) break;
				else if(i==3 && board[x][y].team==board[x+i][y+i].team)goto win;
			}
			for(int i=0;i<4;i++) {//Ditto above, but for vertical lines.
				if(y+i>5||y+i<0)break;
				if(!board[x][y].team) break;
				if(board[x][y].team!=board[x][y+i].team) break;
				else if(i==3 && board[x][y].team==board[x][y+i].team)goto win;
			}
			for(int i=0;i<4;i++) {//Ditto above, but for upward sloping diagonal lines. 
				if(x+i>6||x+i<0)break;
				if(y-i>5||y-i<0)break;
				if(!board[x][y].team) break;
				if(board[x][y].team!=board[x+i][y-i].team) break;
				else if(i==3 && board[x][y].team==board[x+i][y-i].team)goto win;
			}

		}
	}
	return;
	win:
	mode=3;
}

void checkClick(){
	static char lastClick=0;//Stores the state of the mouse on the previous frame.
	int mX;
	int mY;
	if(!SDL_GetMouseState(NULL,NULL)) lastClick=0;
	if(SDL_GetMouseState(&mX,&mY) & SDL_BUTTON(SDL_BUTTON_LEFT) && !lastClick){//If left click is depressed, but was not on the previous frame.
		lastClick=1;//Store the state for the subsequent frame.
		int gridRef=(mX-96)/SIZE;//Finds the column in which the click took place.
		if(gridRef>6 || gridRef<0) return;//If it is outside of the board, return.
		if(board[gridRef][0].team) return;//If the column is full, return.
		for(int i=1;i<6;i++) {
			if(board[gridRef][i].team) {//If the slot is occupied.
				mode=1;//Enable physics.
				board[gridRef][i-1].team=turn;//Set the slot above i as the target for the new block.
				board[gridRef][i-1].y=i*64;//Sets the Y position of the new block above the board.
				checkWin();//Check for a victory state.
				if(turn==1) turn=2;//Hands over to the other player.
				else turn=1;
				break;
			} else if(i==5 && !board[gridRef][5].team) {//If the column is empty.
				mode=1;//Enable physics.
				board[gridRef][i].team=turn;//Set the bottom cell of the column as the target for the new block.
				board[gridRef][i].y=i*64;//Sets the Y position of the new block above the board.
				checkWin();//Check for a victory state.
				if(turn==1) turn=2;//Hands over to the other player.
				else turn=1;
				break;
			}
		}

	}
}

void loop(){
	static int bgScroll=0;//The position of the background layer in its animation.

	if (bgScroll<WIDTH) bgScroll++;//Advances the background layer animation.
	else bgScroll=0;//If the animation is complete, reset it.

	blit(bgLayer,bgScroll,0,WIDTH,HEIGHT);//Draws the background layer twice to provide an illusion of seamlessness.
	blit(bgLayer,-WIDTH+bgScroll,0,WIDTH,HEIGHT);

	switch(mode){//The state machine.
		case 0://The normal mode, in which the game seeks input.
			checkClick();
		break;
		case 1://This mode is used when gravity must be applied to a newly spawned block.
			spriteDrop(1,0);
		break;
		case 3://This mode is used during a victory state to apply gravity to the remaining dangling blocks.
			spriteDrop(3,4);
		break;
		case 4://This state is used following the completion of a game, and prior to the beginning of a new one.
			winLoop();
		break;
	}

	for(int x=0;x<7;x++) {//This loop draws the individual blocks.
		for(int y=0;y<6;y++) {
			if(board[x][y].team) blitSprite(spriteSheet,board[x][y].team-1,x*64+96,y*64+64-board[x][y].y,SIZE,SIZE);
		}
	}
	for(int x=0;x<7;x++) {//This loop draws the board.
		for(int y=0;y<6;y++) {
			blitSprite(spriteSheet,2,x*64+96,y*64+64,64,64);
		}
	}
	blit(TTF_RenderText_Solid(f,"Turn: ",fC),2,0,0,0);//Draws the turn label.
	blitSprite(spriteSheet,turn-1,8,32,0,0);//Draws the block type of the current player.

	SDL_UpdateWindowSurface(w);
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	f=TTF_OpenFont("connectgraph/font.ttf",30);
	fs=TTF_OpenFont("connectgraph/font.ttf",25);
	w=SDL_CreateWindow(TITLE,0,0,WIDTH,HEIGHT,SDL_WINDOW_OPENGL);
	s=SDL_GetWindowSurface(w);
	bgLayer=IMG_Load("connectgraph/bglayer.png");
	spriteSheet=IMG_Load("connectgraph/spritesheet.png");
	memset(&board,0,sizeof board);
	emscripten_set_main_loop(loop,30,1);
	return 0;
}