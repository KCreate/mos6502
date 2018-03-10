#include <SDL2/SDL.h>

int main(int argc, char** argv) {
  SDL_Window* window = NULL;
  window = SDL_CreateWindow("6502 Microcontroller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
                            SDL_WINDOW_SHOWN);

  // Setup renderer
  SDL_Renderer* renderer = NULL;
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_Rect r;
  r.x = 50;
  r.y = 50;
  r.w = 50;
  r.h = 50;

  for (int i = 0; i < 4000; i++) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    r.x = i % 400;
    r.y = i % 400;
    SDL_RenderFillRect(renderer, &r);
    SDL_RenderPresent(renderer);

    SDL_PumpEvents();
    SDL_Delay(1);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return EXIT_SUCCESS;
}
