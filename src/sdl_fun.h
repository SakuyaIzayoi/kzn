#ifndef SDL_FUN_H
#define SDL_FUN_H

#include <SDL2/SDL_video.h>

SDL_Window *createVulkanWindow(int, int, const char *);
void deleteWindow(SDL_Window *);

#endif
