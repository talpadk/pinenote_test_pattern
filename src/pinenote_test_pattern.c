//#exe
//#cflags -D_REENTRANT
//#cflags -I/usr/include/SDL2
//#ldflags -lSDL2

#include <SDL.h>
#include <stdio.h>


SDL_Renderer *renderer_;
SDL_Window *window_;

#define WIDTH (1872)
#define HEIGHT (1404)

static uint8_t keepRunning_ = 1;
static uint8_t needsARedraw_ = 1;
static uint8_t drawDirection_ = 0;

static int rectWidth_ = 42;
static int rectHeight_ = 42;

SDL_Surface *colours_[16];

void initSDL(void) {
  int rendererFlags, windowFlags;
  rendererFlags = SDL_RENDERER_ACCELERATED;
  
  windowFlags = 0;//SDL_WINDOW_FULLSCREEN;
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  window_ = SDL_CreateWindow("PineNote test pattern", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, windowFlags);

  if (window_ == 0){
    printf("Failed to open %d x %d window: %s\n", WIDTH, HEIGHT, SDL_GetError());
    exit(1);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

  renderer_ = SDL_CreateRenderer(window_, -1, rendererFlags);
  
  if (renderer_ == 0) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Rect fillRect;
  fillRect.x = 0;
  fillRect.y = 0;
  fillRect.w = rectWidth_;
  fillRect.h = rectHeight_;
  for (Uint32 i = 0; i < 16; i++) {
    colours_[i] = SDL_CreateRGBSurface(0, rectWidth_, rectHeight_, 24, 0, 0, 0, 0);
    Uint32 colour = (i << (16 + 4)) | (i << (8 + 4)) | (i << (0 + 4));
    SDL_FillRect(colours_[i], &fillRect, colour);
  }
}

static void checkInput(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      keepRunning_ = 0;
      break;
    case SDL_FINGERDOWN:{
	float x = event.tfinger.x;
	float y = event.tfinger.y;
	printf("finger down @ %f,%f\n", x,y);
	drawDirection_ ^= 1;
	needsARedraw_ = 1;
      }
      break;
      case SDL_MOUSEBUTTONDOWN: {
	Sint32 x = event.button.x;
	Sint32 y = event.button.y;
	printf("mouse down @ %d,%d\n", x,y);
	drawDirection_ ^= 1;
	needsARedraw_ = 1;
      }
    }
  }
}

static void drawScreen(void) {
  SDL_Rect sourceRect;
  sourceRect.x = 0;
  sourceRect.y = 0;
  sourceRect.w = rectWidth_;
  sourceRect.h = rectHeight_;
  SDL_Rect destinationRect;
  destinationRect.w = rectWidth_;
  destinationRect.h = rectHeight_;
  SDL_Surface *windowSurface = SDL_GetWindowSurface(window_);
  for (int x=0; x<16; x++){
    for (int y = 0; y < 16; y++) {
        destinationRect.x = x * rectWidth_;
        destinationRect.y = y * rectHeight_;
        int colour;
        if (drawDirection_) {
          colour = y;
        }
	else {
          colour = x;
        }

        SDL_BlitSurface(colours_[colour], &sourceRect, windowSurface, &destinationRect);
    }
  }
  SDL_UpdateWindowSurface(window_);
  needsARedraw_ = 0;
}

int main(int argc, char **args) {
  initSDL();
  while (keepRunning_) {
    checkInput();
    if (needsARedraw_) {
      drawScreen();
    }
  }
  return 0;
}
