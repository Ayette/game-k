#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct
{
    SDL_Texture *background;
    SDL_Texture *image;

    SDL_Rect zone_puzzle;
    SDL_Rect src[9];
    SDL_Rect dest[9];

    SDL_Rect piece_rect[2];
    SDL_Rect piece_start[2];

    int image_actuelle;
    int indices_vides[2];
    int case_remplie[9];
    int side_piece_index[2];
    int side_piece_visible[2];

    int feedback_piece[2];
    Uint32 feedback_timer[2];

    int dragging;
    int selected_piece;
    int offset_x;
    int offset_y;

    int running;
    int termine;
    int perdu;

    Mix_Chunk *sound_win;
    int sound_played;

    SDL_Texture *btn_retour_img;
    SDL_Texture *btn_option_img;
    SDL_Rect btn_retour;
    SDL_Rect btn_option;
    int hover_retour;
    int hover_option;

    TTF_Font *font;

    Mix_Chunk *tick_sound;
    int tick_channel;
    int tick_started;

    Uint32 timer_start;
    Uint32 timer_duration;
    SDL_Rect timer_rect;

    char message[128];
    SDL_Color message_color;
    Uint32 message_timer;
    int show_message;

} puzel;

int  point_dans_rect_puzzle(int x, int y, SDL_Rect r);
int  initialiser_puzzle(puzel *p, SDL_Renderer *renderer);
void generer_puzzle(puzel *p, SDL_Renderer *renderer);
void gerer_evenements_puzzle(puzel *p, SDL_Renderer *renderer, int *etat);
void afficher_puzzle(puzel *p, SDL_Renderer *renderer);
void nettoyer_puzzle(puzel *p);

#endif
