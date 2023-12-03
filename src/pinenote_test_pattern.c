//#exe
//#global_cflags -D_REENTRANT
//#global_cflags -I/usr/include/SDL2
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

static uint8_t useAcceleration_ = 1;
static uint8_t useGlobalRefresh_ = 0;
static uint8_t performTiming_ = 0;
static Uint32  updateDelay_ = 0;

static int rectWidth_ = 42;
static int rectHeight_ = 42;

SDL_Surface *colours_[16];
SDL_Texture *acceleratedColour_[16];
SDL_Texture *offScreenBuffer_;

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

  if (useAcceleration_){
    renderer_ = SDL_CreateRenderer(window_, -1, rendererFlags);

    if (renderer_ == 0) {
      printf("Failed to create renderer: %s\n", SDL_GetError());
      exit(1);
    }

    offScreenBuffer_ =  SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, 16*rectWidth_, 16*rectHeight_);
    if (offScreenBuffer_ == 0) {
      printf("Failed to create off screen rendering texture: %s\n",
             SDL_GetError());
      exit(1);
    }
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
    if (useAcceleration_) {
      acceleratedColour_[i] = SDL_CreateTextureFromSurface(renderer_, colours_[i]);
    }
  }
}

static void checkInput(void) {
  SDL_Event event;
  uint8_t update = 0;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      keepRunning_ = 0;
      break;
    case SDL_FINGERDOWN:{
	float x = event.tfinger.x;
	float y = event.tfinger.y;
	//	printf("finger down @ %f,%f\n", x,y);
	update = 1;
        if (x < 0.1 && y < 0.1) {
          keepRunning_ = 0;
        }
      }
      break;
      case SDL_MOUSEBUTTONDOWN: {
	Sint32 x = event.button.x;
	Sint32 y = event.button.y;
	//printf("mouse down @ %d,%d\n", x,y);
	update = 1;
	if (x < 100 && y < 100) {
          keepRunning_ = 0;
        }
      }
    }
  }
  if (update) {
    drawDirection_ ^= 1;
    needsARedraw_ = 1;
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
  if (useAcceleration_) {
    SDL_SetRenderTarget(renderer_, offScreenBuffer_);
  }

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
      if (useAcceleration_) {
        if (SDL_RenderCopy(renderer_, acceleratedColour_[colour], 0,
                           &destinationRect) != 0) {
          printf("SDL error '%s' while accelerated rendering @ tile %u,%u\n",
                 SDL_GetError(), x, y);
        }
      } else {
          SDL_BlitSurface(colours_[colour], &sourceRect, windowSurface,
                          &destinationRect);
      }
      if (!useGlobalRefresh_){
	if (useAcceleration_) {
	  SDL_Rect all;
	  all.x = 0;
	  all.y = 0;
	  all.w = 16*rectWidth_;
	  all.h = 16*rectHeight_;
	  SDL_SetRenderTarget(renderer_, NULL);
	  SDL_RenderCopy(renderer_, offScreenBuffer_, NULL, &all);
	  SDL_RenderPresent(renderer_);
	  SDL_SetRenderTarget(renderer_, offScreenBuffer_);
	  //SDL_RenderClear(renderer_);
	}
	else {
	  SDL_UpdateWindowSurfaceRects(window_, &destinationRect, 1);
	}
        if (updateDelay_ != 0) {
          SDL_Delay(updateDelay_);
        }
      }
    }
  }

  if (useAcceleration_) {
    SDL_Rect all;
    all.x = 0;
    all.y = 0;
    all.w = 16*rectWidth_;
    all.h = 16*rectHeight_;
    SDL_SetRenderTarget(renderer_, NULL);
    SDL_RenderCopy(renderer_, offScreenBuffer_, NULL, &all);
    SDL_RenderPresent(renderer_);
  }
  else if (useGlobalRefresh_) {
    destinationRect.x = 0;
    destinationRect.y = 0;
    destinationRect.w = 16 * rectWidth_;
    destinationRect.h = 16 * rectHeight_;
    SDL_UpdateWindowSurfaceRects(window_, &destinationRect, 1);
  }
  
  if (updateDelay_ != 0) {
    SDL_Delay(updateDelay_);
  }
  needsARedraw_ = 0;
}

static void showHelp(void) {
  printf("Usage: pinenote_test_pattern [option]\n\n");
  printf("  -h\tShow this help\n");
  printf("  -a\tDisable HW accelerated rendering\n");
  printf("  -d=X\tDraw X squares per update call, valid values 1,256\n");
  printf("  -p=X\tInsert a pause of X ms after each update call\n");
  printf("  -t\tShow timing statistics\n");
  printf("\n");
  printf("In the 'application' press the upper left corner to exit, press anywhere else to draw the 'next' test pattern\n\n");
}

static uint8_t readArguments(int argCount, const char **arguments) {
  uint8_t run = 1;
  uint8_t help = 0;
  for (int i = 0; i < argCount; i++) {
    const char *argument = arguments[i];
    if (i == 0) {
    }    // skip name
    else if (strcmp(argument, "-h") == 0) {
      help = 1;
    }
    else if (strcmp(argument, "-a") == 0) {
      useAcceleration_ = 0;
    }
    else if (strcmp(argument, "-t") == 0) {
      performTiming_ = 1;
    }
    else if (strncmp(argument, "-p=", 3) == 0) {
      uint32_t number = 0;
      const char *character = argument+3;
      while (*character >= '0' && *character <= '9') {
        number = number * 10 + (*character - '0');
        character++;
      }
      updateDelay_ = number;
    } else if (strncmp(argument, "-d=", 3) == 0) {
      uint32_t number = 0;
      const char *character = argument+3;
      while (*character >= '0' && *character <= '9') {
	number = number*10 + (*character - '0');
	character++;
      }
      if (number == 1) {
        useGlobalRefresh_ = 0;
      }
      else if (number == 256) {
        useGlobalRefresh_ = 1;
      } else {
        help = 1;
      }
    }
    else {
      help = 1;
    }
  }

  if (help) {
    showHelp();
    run = 0;
  }

  return run;
}

static void showSettingsUsed(void){
  if (useAcceleration_) {
    printf("Drawing using acceleration\n");
  }
  else {
    printf("Drawing without acceleration\n");
  }

  if (useGlobalRefresh_) {
    printf("Performing the update of the screen as a single update\n");
  }
  else {
    printf("Performing the update of the screen as 256 updates\n");
  }

  if (updateDelay_ == 0) {
    printf("The updates will be performed as fast as possible\n");
  }
  else {
    printf("After each update there will be inserted a %ums pause\n", updateDelay_);
  }

  if (performTiming_) {
    printf("The time for each redraw will be written\n");
  }
  else {
    printf("No timing statistics will be provided\n");
  }

  printf("\n");
}

int main(int argc, const char **args) {
  if (readArguments(argc, args)) {
    showSettingsUsed();
    initSDL();
    needsARedraw_ = 1;
    while (keepRunning_) {
      if (needsARedraw_) {
	Uint64 start = SDL_GetTicks64();
        drawScreen();
	Uint64 end = SDL_GetTicks64();
        if (performTiming_) {
	  printf("Draw took %lums\n", end-start);
        }
      }
      checkInput();
    }
  }
  return 0;
}
