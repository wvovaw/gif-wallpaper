#define SDL_MAIN_HANDLED
#include <SDL.h>
//#undef main

#include <stdbool.h>

#include "paperview.h" 

int main(int argc, char *argv[])
{
	SDL_SetMainReady();
	Video video = Setup();
    View* views = Parse(argc, argv, &video);
    bool running = true;
	int frame = 0;
	while (running)
	{ 
        for (View* view = views; view; view = view->next)
            SDL_RenderCopy(video.renderer, view->textures.texture[frame], NULL, view->rect);
        SDL_RenderPresent(video.renderer);

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						running = false;
						break;
				}
				break;
			}
		}
        frame == views->textures.size - 1 ? frame = 0 : ++frame;
		SDL_Delay(views->delay);
	}
    Cleanup(views);
    Teardown(&video);
	SDL_Quit();
    exit(0);
}
