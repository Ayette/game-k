#ifndef OPTION_H
#define OPTION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

typedef struct
{
    SDL_Texture *texture;
    SDL_Rect rect;
    int hover;
} Bouton;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;

    Bouton btn_plus;
    Bouton btn_minus;
    Bouton btn_normal;
    Bouton btn_fullscreen;
    Bouton btn_retour;

    Mix_Music *music;
    Mix_Chunk *hover_sound;

    int running;
    int volume;
    int fullscreen;
} Option;

int initialiser_option(Option *o);
void gerer_evenements_option(Option *o, int *etat);
void afficher_option(Option *o);
void nettoyer_option(Option *o);

#endif
