#ifndef PAPERVIEW_H_
#define PAPERVIEW_H_

#include <SDL.h>

typedef struct
{
    char **path;
    unsigned size;
} Paths;

Paths Populate(const char *);
void Depopulate(Paths *);

typedef struct
{
    SDL_Texture **texture;
    unsigned size;
} Textures;

Textures Cache(Paths *, SDL_Renderer *);
void Destroy(Textures *);

typedef struct View
{
    int delay;
    Textures textures;
    SDL_Rect *rect;
    struct View *next;
} View;

View *Init(const char *const, const int, SDL_Rect *, struct Video *);

View *Push(View *, View *);
View *Parse(int, char **, struct Video *);
void Cleanup(View *);

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} Video;

Video Setup(void);
void Teardown(Video *);

void Quit(const char *const, ...);
int Compare(const void *, const void *);
void Sort(Paths *);

#endif // PAPERVIEW_H_
