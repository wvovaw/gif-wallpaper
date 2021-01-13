#include <SDL.h>
//#undef main

#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <climits>

#include "paperview.h"
#include "wallpaper.h"
#include "error.h"


void Quit(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
    exit(1);
}

int Compare(const void* a, const void* b)
{
    char* const pa = *(char**)a;
    char* const pb = *(char**)b;
    const unsigned la = strlen(pa);
    const unsigned lb = strlen(pb);
    return (la > lb) ? 1 : (la < lb) ? -1 : strcmp(pa, pb);
}

void Sort(Paths* self)
{
    qsort(self->path, self->size, sizeof(*self->path), Compare);
}

Paths Populate(const char* base)
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

void Depopulate(Paths* self)
{
    for (unsigned i = 0; i < self->size; i++)
        free(self->path[i]);
    free(self->path);
}

Textures Cache(Paths* paths, SDL_Renderer* renderer)
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

void Destroy(Textures* self)
{
    for (unsigned i = 0; i < self->size; i++)
        SDL_DestroyTexture(self->texture[i]);
    free(self->texture);
}

Video Setup(void)
{
    Video self;
    self.window = create_wallpaper_window("gif-wallpaper");
    if (self.window == NULL)
    {
        fprintf(stderr, "An error occured while creating the wallpaper window:\n%s\n", get_last_error());
        SDL_Quit();
        SDL_DestroyWindow(self.window);
        exit(1);
    }

    self.renderer = SDL_CreateRenderer(self.window, -1, SDL_RENDERER_ACCELERATED);
    if (self.renderer == NULL)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_Quit();
        SDL_DestroyWindow(self.window);
        SDL_DestroyRenderer(self.renderer);
        exit(1);
    }

    SDL_ShowWindow(self.window);
    SDL_Init(SDL_INIT_EVERYTHING);
    return self;
}

void Teardown(Video* self)
{
    SDL_Quit();
    SDL_DestroyWindow(self->window);
    SDL_DestroyRenderer(self->renderer);
}

View* Init(const char* const base, const int delay, SDL_Rect* rect, Video* video)
{
    View* self = malloc(sizeof(*self));
    self->delay = delay;
    Paths paths = Populate(base);
    self->textures = Cache(&paths, video->renderer);
    self->rect = rect;
    Depopulate(&paths);
    self->next = NULL;
    return self;
}

View* Push(View* views, View* view)
{
    view->next = views;
    return view;
}

void Cleanup(View* views)
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

View* Parse(int argc, char** argv, Video* video)
{
    const int args = argc - 1;
    if (args < 2)
        Quit("Usage: gif-wallpaper.exe FOLDER DELAY\n");
    const int params = 6;
    if (args > 2 && args % params != 0)
        Quit("Usage: gif-wallpaper.exe FOLDER DELAY X Y W H FOLDER DELAY X Y W H # ... And so on\n"); // MULTI-MONITOR PARAMETER SUPPORT.
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
        int delay = atoi(argv[b]);
        if (delay == 0)
            Quit("Invalid delay value\n");
        if (delay < 0)
            delay = MAXINT32; // NEGATIVE DELAY VALUES CREATE STILL WALLPAPERS.
        SDL_Rect* rect = NULL;
        if (c != argc)
        {
            rect = malloc(sizeof(*rect));
            rect->x = atoi(argv[c]);
            rect->y = atoi(argv[d]);
            rect->w = atoi(argv[e]);
            rect->h = atoi(argv[f]);
        }
        views = Push(views, Init(base, delay, rect, video));
    }
    return views;
}
