#include <SDL.h>
#include <dirent.h>
#include <stdarg.h>
#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <climits>

#include "wallpaper.h"
#include "error.h"

typedef struct
{
    char** path;
    unsigned size;
}
Paths;

typedef struct
{
    SDL_Texture** texture;
    unsigned size;
}
Textures;

typedef struct View
{
/* 
*  In this version of paperview we call SDL_Delay with SPEED parameter.
*  So, the SPEED parameter is the actual delay time between rendering of two adjacent frames.
*/
    int speed;
    Textures textures;
    SDL_Rect* rect;
    struct View* next;
}
View;

typedef struct
{
    SDL_Window* window;
    SDL_Renderer* renderer;
}
Video;

static void Quit(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
    exit(1);
}

static int Compare(const void* a, const void* b)
{
    char* const pa = *(char**)a;
    char* const pb = *(char**)b;
    const unsigned la = strlen(pa);
    const unsigned lb = strlen(pb);
    return (la > lb) ? 1 : (la < lb) ? -1 : strcmp(pa, pb);
}

static void Sort(Paths* self)
{
    qsort(self->path, self->size, sizeof(*self->path), Compare);
}

static Paths Populate(const char* base)
{
    DIR* const dir = opendir(base);
    if (dir == NULL)
        Quit("Directory '%s' failed to open\n", base);
    unsigned max = 8;
    Paths self;
    self.size = 0;
    self.path = malloc(max * sizeof(*self.path));
    for (struct dirent* entry; (entry = readdir(dir));)
    {
        const char* const path = entry->d_name;
        if (strstr(path, ".bmp"))
        {
            char* const slash = "/";
            char* const buffer = malloc(strlen(base) + strlen(slash) + strlen(path) + 1);
            strcpy(buffer, base);
            strcat(buffer, slash);
            strcat(buffer, path);
            if (self.size == max)
            {
                max *= 2;
                self.path = realloc(self.path, max * sizeof(*self.path));
            }
            self.path[self.size] = buffer;
            self.size += 1;
        }
    }
    closedir(dir);
    Sort(&self);
    return self;
}

static void Depopulate(Paths* self)
{
    for (unsigned i = 0; i < self->size; i++)
        free(self->path[i]);
    free(self->path);
}

static Textures Cache(Paths* paths, SDL_Renderer* renderer)
{
    Textures self;
    self.size = paths->size;
    self.texture = malloc(self.size * sizeof(*self.texture));
    for (unsigned i = 0; i < self.size; i++)
    {
        const char* const path = paths->path[i];
        SDL_Surface* const surface = SDL_LoadBMP(path);
        if (surface == NULL)
            Quit("File '%s' failed to open. %s\n", path, SDL_GetError());
        self.texture[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    return self;
}

static void Destroy(Textures* self)
{
    for (unsigned i = 0; i < self->size; i++)
        SDL_DestroyTexture(self->texture[i]);
    free(self->texture);
}

static Video Setup(void)
{
    Video self;
	self.window = create_wallpaper_window("sdl-wallpaper");
	if (self.window == NULL)
	{
		fprintf(stderr, "An error occured while creating the wallpaper window:\n%s\n", get_last_error());
		goto fail_create_window;
	}

	self.renderer = SDL_CreateRenderer(self.window, -1, SDL_RENDERER_ACCELERATED);
	if (self.renderer == NULL)
	{
		fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
		goto fail_create_renderer;
	}

	SDL_ShowWindow(self.window);

    SDL_Init(SDL_INIT_EVERYTHING);
    return self;

fail_create_renderer:
	SDL_DestroyWindow(self.window);
fail_create_window:
	SDL_Quit();
    return self;
}

static void Teardown(Video* self)
{
    SDL_Quit();
    SDL_DestroyWindow(self->window);
    SDL_DestroyRenderer(self->renderer);
}

static View* Init(const char* const base, const int speed, SDL_Rect* rect, Video* video)
{
    View* self = malloc(sizeof(*self));
    self->speed = speed;
    Paths paths = Populate(base);
    self->textures = Cache(&paths, video->renderer);
    self->rect = rect;
    Depopulate(&paths);
    self->next = NULL;
    return self;
}

static View* Push(View* views, View* view)
{
    view->next = views;
    return view;
}

static void Cleanup(View* views)
{
    View* view = views;
    while (view)
    {
        View* next = view->next;
        Destroy(&view->textures);
        free(view->rect);
        free(view);
        view = next;
    }
}

static View* Parse(int argc, char** argv, Video* video)
{
    const int args = argc - 1;
    if (args < 2)
        Quit("Usage: paperview FOLDER SPEED\n"); // Here SPEED is ms delay between frames
    const int params = 6;
    if (args > 2 && args % params != 0)
        Quit("Usage: paperview FOLDER SPEED X Y W H FOLDER SPEED X Y W H # ... And so on\n"); // MULTI-MONITOR PARAMETER SUPPORT.
    View* views = NULL;
    for (int i = 1; i < argc; i += params)
    {
        const int a = i + 0;
        const int b = i + 1;
        const int c = i + 2;
        const int d = i + 3;
        const int e = i + 4;
        const int f = i + 5;
        const char* const base = argv[a];
        int speed = atoi(argv[b]);
        if (speed == 0)
            Quit("Invalid speed value\n");
        if (speed < 0)
            speed = MAXINT32; // NEGATIVE SPEED VALUES CREATE STILL WALLPAPERS.
        SDL_Rect* rect = NULL;
        if (c != argc)
        {
            rect = malloc(sizeof(*rect));
            rect->x = atoi(argv[c]);
            rect->y = atoi(argv[d]);
            rect->w = atoi(argv[e]);
            rect->h = atoi(argv[f]);
        }
        views = Push(views, Init(base, speed, rect, video));
    }
    return views;
}


int main(int argc, char *argv[])
{
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
		SDL_Delay(views->speed);
	}
    Cleanup(views);
    Teardown(&video);
	SDL_Quit();
	return 0;
}
