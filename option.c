#include "option.h"

int point_dans_rect(int x, int y, SDL_Rect r)
{
    if (x >= r.x && x <= r.x + r.w &&
        y >= r.y && y <= r.y + r.h)
        return 1;
    return 0;
}

void mettre_a_jour_layout_option(Option *o)
{
    int w, h;
    float sx, sy;

    SDL_GetWindowSize(o->window, &w, &h);

    sx = (float)w / 1200.0f;
    sy = (float)h / 800.0f;

    o->btn_minus.rect.x = (int)(545 * sx);
    o->btn_minus.rect.y = (int)(315 * sy);
    o->btn_minus.rect.w = (int)(115 * sx);
    o->btn_minus.rect.h = (int)(42 * sy);

    o->btn_plus.rect.x = (int)(710 * sx);
    o->btn_plus.rect.y = (int)(315 * sy);
    o->btn_plus.rect.w = (int)(115 * sx);
    o->btn_plus.rect.h = (int)(42 * sy);

    o->btn_normal.rect.x = (int)(545 * sx);
    o->btn_normal.rect.y = (int)(450 * sy);
    o->btn_normal.rect.w = (int)(115 * sx);
    o->btn_normal.rect.h = (int)(42 * sy);

    o->btn_fullscreen.rect.x = (int)(710 * sx);
    o->btn_fullscreen.rect.y = (int)(450 * sy);
    o->btn_fullscreen.rect.w = (int)(135 * sx);
    o->btn_fullscreen.rect.h = (int)(42 * sy);

    o->btn_retour.rect.x = (int)(710 * sx);
    o->btn_retour.rect.y = (int)(575 * sy);
    o->btn_retour.rect.w = (int)(105 * sx);
    o->btn_retour.rect.h = (int)(38 * sy);
}

void dessiner_bouton(Option *o, Bouton b)
{
    SDL_RenderCopy(o->renderer, b.texture, NULL, &b.rect);

    if (b.hover)
    {
        SDL_SetRenderDrawColor(o->renderer, 255, 215, 0, 255);
        SDL_RenderDrawRect(o->renderer, &b.rect);

        SDL_Rect r2 = {b.rect.x - 2, b.rect.y - 2, b.rect.w + 4, b.rect.h + 4};
        SDL_RenderDrawRect(o->renderer, &r2);
    }
}

int initialiser_option(Option *o)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 0;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        return 0;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());
        return 0;
    }

    o->window = SDL_CreateWindow("Sous Menu Option",
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 1200, 800,
                                 SDL_WINDOW_SHOWN);
    if (o->window == NULL)
    {
        printf("Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        return 0;
    }

    o->renderer = SDL_CreateRenderer(o->window, -1,
                                     SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC);
    if (o->renderer == NULL)
    {
        printf("Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        return 0;
    }

    o->background = IMG_LoadTexture(o->renderer, "assets/images/bg2.png");
    if (o->background == NULL)
    {
        printf("Erreur chargement assets/images/bg2.png : %s\n", IMG_GetError());
        return 0;
    }

    o->btn_plus.texture = IMG_LoadTexture(o->renderer, "assets/images/plus.png");
    if (o->btn_plus.texture == NULL)
    {
        printf("Erreur chargement plus.png : %s\n", IMG_GetError());
        return 0;
    }

    o->btn_minus.texture = IMG_LoadTexture(o->renderer, "assets/images/minus.png");
    if (o->btn_minus.texture == NULL)
    {
        printf("Erreur chargement minus.png : %s\n", IMG_GetError());
        return 0;
    }

    o->btn_normal.texture = IMG_LoadTexture(o->renderer, "assets/images/normal.png");
    if (o->btn_normal.texture == NULL)
    {
        printf("Erreur chargement normal.png : %s\n", IMG_GetError());
        return 0;
    }

    o->btn_fullscreen.texture = IMG_LoadTexture(o->renderer, "assets/images/fullscreen.png");
    if (o->btn_fullscreen.texture == NULL)
    {
        printf("Erreur chargement fullscreen.png : %s\n", IMG_GetError());
        return 0;
    }

    o->btn_retour.texture = IMG_LoadTexture(o->renderer, "assets/images/retour.png");
    if (o->btn_retour.texture == NULL)
    {
        printf("Erreur chargement retour.png : %s\n", IMG_GetError());
        return 0;
    }

    o->music = Mix_LoadMUS("assets/sounds/music2.mp3");
    if (o->music == NULL)
        printf("Warning music2.mp3 : %s\n", Mix_GetError());

    o->hover_sound = Mix_LoadWAV("assets/sounds/hover.wav");
    if (o->hover_sound == NULL)
        printf("Warning hover.wav : %s\n", Mix_GetError());

    o->btn_plus.hover = 0;
    o->btn_minus.hover = 0;
    o->btn_normal.hover = 0;
    o->btn_fullscreen.hover = 0;
    o->btn_retour.hover = 0;

    o->running = 1;
    o->volume = 64;
    o->fullscreen = 0;

    Mix_VolumeMusic(o->volume);

    if (o->music != NULL)
        Mix_PlayMusic(o->music, -1);

    mettre_a_jour_layout_option(o);

    return 1;
}

void gerer_evenements_option(Option *o, int *etat)
{
    SDL_Event e;
    int mx, my;
    int old_hover_plus, old_hover_minus, old_hover_normal, old_hover_fullscreen, old_hover_retour;

    mettre_a_jour_layout_option(o);

    SDL_GetMouseState(&mx, &my);

    old_hover_plus = o->btn_plus.hover;
    old_hover_minus = o->btn_minus.hover;
    old_hover_normal = o->btn_normal.hover;
    old_hover_fullscreen = o->btn_fullscreen.hover;
    old_hover_retour = o->btn_retour.hover;

    o->btn_plus.hover = point_dans_rect(mx, my, o->btn_plus.rect);
    o->btn_minus.hover = point_dans_rect(mx, my, o->btn_minus.rect);
    o->btn_normal.hover = point_dans_rect(mx, my, o->btn_normal.rect);
    o->btn_fullscreen.hover = point_dans_rect(mx, my, o->btn_fullscreen.rect);
    o->btn_retour.hover = point_dans_rect(mx, my, o->btn_retour.rect);

    if (o->hover_sound != NULL)
    {
        if (o->btn_plus.hover && !old_hover_plus) Mix_PlayChannel(-1, o->hover_sound, 0);
        if (o->btn_minus.hover && !old_hover_minus) Mix_PlayChannel(-1, o->hover_sound, 0);
        if (o->btn_normal.hover && !old_hover_normal) Mix_PlayChannel(-1, o->hover_sound, 0);
        if (o->btn_fullscreen.hover && !old_hover_fullscreen) Mix_PlayChannel(-1, o->hover_sound, 0);
        if (o->btn_retour.hover && !old_hover_retour) Mix_PlayChannel(-1, o->hover_sound, 0);
    }

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            o->running = 0;

        if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.sym == SDLK_ESCAPE)
                o->running = 0;

            if (e.key.keysym.sym == SDLK_PLUS ||
                e.key.keysym.sym == SDLK_KP_PLUS ||
                e.key.keysym.sym == SDLK_EQUALS)
            {
                if (o->volume < 128) o->volume += 8;
                if (o->volume > 128) o->volume = 128;
                Mix_VolumeMusic(o->volume);
            }

            if (e.key.keysym.sym == SDLK_MINUS ||
                e.key.keysym.sym == SDLK_KP_MINUS)
            {
                if (o->volume > 0) o->volume -= 8;
                if (o->volume < 0) o->volume = 0;
                Mix_VolumeMusic(o->volume);
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            SDL_GetMouseState(&mx, &my);

            if (point_dans_rect(mx, my, o->btn_plus.rect))
            {
                if (o->volume < 128) o->volume += 8;
                if (o->volume > 128) o->volume = 128;
                Mix_VolumeMusic(o->volume);
            }

            if (point_dans_rect(mx, my, o->btn_minus.rect))
            {
                if (o->volume > 0) o->volume -= 8;
                if (o->volume < 0) o->volume = 0;
                Mix_VolumeMusic(o->volume);
            }

            if (point_dans_rect(mx, my, o->btn_normal.rect))
            {
                SDL_SetWindowFullscreen(o->window, 0);
                SDL_SetWindowSize(o->window, 1200, 800);
                SDL_SetWindowPosition(o->window,
                                      SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED);
                o->fullscreen = 0;
                mettre_a_jour_layout_option(o);
            }

            if (point_dans_rect(mx, my, o->btn_fullscreen.rect))
            {
                SDL_SetWindowFullscreen(o->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                o->fullscreen = 1;
                mettre_a_jour_layout_option(o);
            }

            if (point_dans_rect(mx, my, o->btn_retour.rect))
            {
                *etat = 2;
            }
        }
    }
}

void afficher_option(Option *o)
{
    SDL_Rect bg_rect;
    int w, h;

    SDL_GetWindowSize(o->window, &w, &h);

    bg_rect.x = 0;
    bg_rect.y = 0;
    bg_rect.w = w;
    bg_rect.h = h;

    SDL_RenderClear(o->renderer);
    SDL_RenderCopy(o->renderer, o->background, NULL, &bg_rect);

    dessiner_bouton(o, o->btn_minus);
    dessiner_bouton(o, o->btn_plus);
    dessiner_bouton(o, o->btn_normal);
    dessiner_bouton(o, o->btn_fullscreen);
    dessiner_bouton(o, o->btn_retour);

    SDL_RenderPresent(o->renderer);
}

void nettoyer_option(Option *o)
{
    if (o->music != NULL)
        Mix_FreeMusic(o->music);

    if (o->hover_sound != NULL)
        Mix_FreeChunk(o->hover_sound);

    if (o->btn_plus.texture != NULL)
        SDL_DestroyTexture(o->btn_plus.texture);

    if (o->btn_minus.texture != NULL)
        SDL_DestroyTexture(o->btn_minus.texture);

    if (o->btn_normal.texture != NULL)
        SDL_DestroyTexture(o->btn_normal.texture);

    if (o->btn_fullscreen.texture != NULL)
        SDL_DestroyTexture(o->btn_fullscreen.texture);

    if (o->btn_retour.texture != NULL)
        SDL_DestroyTexture(o->btn_retour.texture);

    if (o->background != NULL)
        SDL_DestroyTexture(o->background);

    if (o->renderer != NULL)
        SDL_DestroyRenderer(o->renderer);

    if (o->window != NULL)
        SDL_DestroyWindow(o->window);

    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}
