//#exe
//#cflags -D_REENTRANT
//#cflags -I/usr/include/SDL2
//#ldflags -lSDL2

#include  <SDL.h>


SDL_Renderer *renderer_;
SDL_Window *window_;

#define WIDTH (1872)
#define HEIGHT (1404)


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
}

int main(int argc, char **args) {
  initSDL();
  return 0;
}
