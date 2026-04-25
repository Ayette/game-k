#include "puzzle.h"

#define FEEDBACK_MS 600
#define MESSAGE_MS  2000
#define ETAT_OPTION 0

static void set_message(puzel *p, const char *texte, SDL_Color color)
{
    snprintf(p->message, sizeof(p->message), "%s", texte);
    p->message_color = color;
    p->message_timer = SDL_GetTicks();
    p->show_message = 1;
}

static void afficher_texte(SDL_Renderer *renderer, TTF_Font *font,
                           const char *texte, SDL_Color color, int x, int y)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    if (!font || !texte || texte[0] == '\0')
        return;

    surface = TTF_RenderUTF8_Blended(font, texte, color);
    if (!surface)
        return;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        SDL_FreeSurface(surface);
        return;
    }

    rect.x = x;
    rect.y = y;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

static void dessiner_bouton_image(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect rect, int hover)
{
    if (!texture)
        return;

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    if (hover)
    {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);

        SDL_Rect r2 = {rect.x - 2, rect.y - 2, rect.w + 4, rect.h + 4};
        SDL_RenderDrawRect(renderer, &r2);
    }
}

static void afficher_timer_animation(SDL_Renderer *renderer, puzel *p)
{
    int elapsed, remaining;
    float ratio;
    int levels;
    int i;

    SDL_Rect frame;
    SDL_Rect inner;
    SDL_Rect fill;

    if (!p->tick_started || p->termine || p->perdu)
        return;

    elapsed = SDL_GetTicks() - p->timer_start;
    remaining = (int)p->timer_duration - elapsed;

    if (remaining < 0)
        remaining = 0;

    ratio = (float)remaining / (float)p->timer_duration;

    levels = (int)(ratio * 6.0f + 0.99f);
    if (levels < 0) levels = 0;
    if (levels > 6) levels = 6;

    frame = p->timer_rect;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &frame);

    inner.x = frame.x + 14;
    inner.y = frame.y + 14;
    inner.w = frame.w - 28;
    inner.h = frame.h - 28;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &inner);

    if (levels > 0)
    {
        int part_w = inner.w / 6;

        for (i = 0; i < levels; i++)
        {
            fill.x = inner.x + i * part_w;
            fill.y = inner.y;
            fill.w = part_w;
            fill.h = inner.h;

            SDL_SetRenderDrawColor(renderer, 200 + i * 8, 45 + i * 8, 0, 255);
            SDL_RenderFillRect(renderer, &fill);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &frame);
}

static void mettre_a_jour_layout_puzzle(puzel *p, SDL_Renderer *renderer)
{
    int w, h;
    float sx, sy;
    int w_piece, h_piece;

    SDL_GetRendererOutputSize(renderer, &w, &h);

    sx = (float)w / 1200.0f;
    sy = (float)h / 800.0f;

    p->btn_retour.x = (int)(710 * sx);
    p->btn_retour.y = (int)(575 * sy);
    p->btn_retour.w = (int)(105 * sx);
    p->btn_retour.h = (int)(38 * sy);

    p->btn_option.w = (int)(85 * sx);
    p->btn_option.h = (int)(85 * sy);
    p->btn_option.x = (int)(1090 * sx);
    p->btn_option.y = (int)(25 * sy);

    p->timer_rect.x = (int)(40 * sx);
    p->timer_rect.y = (int)(60 * sy);
    p->timer_rect.w = (int)(260 * sx);
    p->timer_rect.h = (int)(55 * sy);

    w_piece = p->zone_puzzle.w / 3;
    h_piece = p->zone_puzzle.h / 3;

    if (p->side_piece_visible[0] && !(p->dragging && p->selected_piece == 0))
    {
        p->piece_rect[0].x = (int)(930 * sx);
        p->piece_rect[0].y = (int)(200 * sy);
        p->piece_rect[0].w = w_piece;
        p->piece_rect[0].h = h_piece;
        p->piece_start[0] = p->piece_rect[0];
    }

    if (p->side_piece_visible[1] && !(p->dragging && p->selected_piece == 1))
    {
        p->piece_rect[1].x = (int)(930 * sx);
        p->piece_rect[1].y = (int)(200 * sy) + h_piece + (int)(30 * sy);
        p->piece_rect[1].w = w_piece;
        p->piece_rect[1].h = h_piece;
        p->piece_start[1] = p->piece_rect[1];
    }
}

static void verifier_fin_tick(puzel *p)
{
    Uint32 now;

    if (p->termine || p->perdu)
        return;

    if (p->tick_started)
    {
        now = SDL_GetTicks();

        if (now - p->timer_start >= p->timer_duration)
        {
            if (p->tick_channel != -1)
                Mix_HaltChannel(p->tick_channel);

            p->perdu = 1;
            p->dragging = 0;
            p->selected_piece = -1;
            p->tick_started = 0;
            p->tick_channel = -1;

            set_message(p, "Temps ecoule !", (SDL_Color){220, 0, 0, 255});
        }
    }
}

static void lancer_tick_si_necessaire(puzel *p)
{
    if (p->termine || p->perdu)
        return;

    if (!p->tick_started)
    {
        p->timer_start = SDL_GetTicks();

        if (p->tick_sound)
            p->tick_channel = Mix_PlayChannel(-1, p->tick_sound, 0);

        p->tick_started = 1;
    }
}

int point_dans_rect_puzzle(int x, int y, SDL_Rect r)
{
    return (x >= r.x && x <= r.x + r.w &&
            y >= r.y && y <= r.y + r.h);
}

int initialiser_puzzle(puzel *p, SDL_Renderer *renderer)
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());

    if (TTF_Init() == -1)
        printf("Erreur TTF_Init : %s\n", TTF_GetError());

    p->background = IMG_LoadTexture(renderer, "assets/images/bg2.png");
    if (!p->background)
    {
        printf("Erreur bg2 puzzle : %s\n", IMG_GetError());
        return 0;
    }

    p->sound_win = Mix_LoadWAV("assets/sounds/win.wav");
    if (!p->sound_win)
        printf("Erreur son win : %s\n", Mix_GetError());

    p->tick_sound = Mix_LoadWAV("assets/sounds/tick.wav");
    if (!p->tick_sound)
        printf("Erreur chargement tick.wav : %s\n", Mix_GetError());

    p->font = TTF_OpenFont("assets/fonts/arial.ttf", 28);
    if (!p->font)
        p->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 28);

    p->btn_retour_img = IMG_LoadTexture(renderer, "assets/images/retour.png");
    if (!p->btn_retour_img)
        printf("Erreur chargement retour.png : %s\n", IMG_GetError());

    p->btn_option_img = IMG_LoadTexture(renderer, "assets/images/option.png");
    if (!p->btn_option_img)
        printf("Erreur chargement option.png : %s\n", IMG_GetError());

    p->image = NULL;
    p->dragging = 0;
    p->selected_piece = -1;
    p->offset_x = 0;
    p->offset_y = 0;
    p->running = 1;
    p->termine = 0;
    p->perdu = 0;
    p->sound_played = 0;

    p->hover_retour = 0;
    p->hover_option = 0;

    p->tick_channel = -1;
    p->tick_started = 0;

    p->timer_start = 0;
    p->timer_duration = 11000;

    p->timer_rect.x = 40;
    p->timer_rect.y = 60;
    p->timer_rect.w = 260;
    p->timer_rect.h = 55;

    p->show_message = 0;
    p->message[0] = '\0';

    p->zone_puzzle.x = 260;
    p->zone_puzzle.y = 140;
    p->zone_puzzle.w = 450;
    p->zone_puzzle.h = 450;

    generer_puzzle(p, renderer);
    mettre_a_jour_layout_puzzle(p, renderer);

    return 1;
}

void generer_puzzle(puzel *p, SDL_Renderer *renderer)
{
    int i, r;
    int img_w, img_h, new_w, new_h, w_piece, h_piece;
    float scale_x, scale_y, scale;
    char chemin[128];

    if (p->image)
        SDL_DestroyTexture(p->image);

    if (p->tick_channel != -1)
        Mix_HaltChannel(p->tick_channel);

    r = rand() % 3;
    p->image_actuelle = r;
    sprintf(chemin, "assets/images/p%d.png", r + 1);

    p->image = IMG_LoadTexture(renderer, chemin);
    if (!p->image)
    {
        printf("Erreur image puzzle : %s\n", IMG_GetError());
        return;
    }

    SDL_QueryTexture(p->image, NULL, NULL, &img_w, &img_h);

    scale_x = 450.0f / img_w;
    scale_y = 450.0f / img_h;
    scale = (scale_x < scale_y) ? scale_x : scale_y;

    new_w = (int)(img_w * scale);
    new_h = (int)(img_h * scale);

    p->zone_puzzle.x = (1200 - new_w) / 2;
    p->zone_puzzle.y = (800 - new_h) / 2;
    p->zone_puzzle.w = new_w;
    p->zone_puzzle.h = new_h;

    w_piece = new_w / 3;
    h_piece = new_h / 3;

    for (i = 0; i < 9; i++)
    {
        p->src[i].x = (i % 3) * (img_w / 3);
        p->src[i].y = (i / 3) * (img_h / 3);
        p->src[i].w = img_w / 3;
        p->src[i].h = img_h / 3;

        p->dest[i].x = p->zone_puzzle.x + (i % 3) * w_piece;
        p->dest[i].y = p->zone_puzzle.y + (i / 3) * h_piece;
        p->dest[i].w = w_piece;
        p->dest[i].h = h_piece;

        p->case_remplie[i] = 1;
    }

    p->indices_vides[0] = rand() % 9;
    do { p->indices_vides[1] = rand() % 9; }
    while (p->indices_vides[1] == p->indices_vides[0]);

    p->case_remplie[p->indices_vides[0]] = 0;
    p->case_remplie[p->indices_vides[1]] = 0;

    {
        int ordre[2] = {0, 1};
        if (rand() % 2)
        {
            ordre[0] = 1;
            ordre[1] = 0;
        }
        p->side_piece_index[0] = p->indices_vides[ordre[0]];
        p->side_piece_index[1] = p->indices_vides[ordre[1]];
    }

    p->side_piece_visible[0] = 1;
    p->side_piece_visible[1] = 1;

    p->feedback_piece[0] = 0;
    p->feedback_piece[1] = 0;
    p->feedback_timer[0] = 0;
    p->feedback_timer[1] = 0;

    p->piece_rect[0].x = 930;
    p->piece_rect[0].y = 200;
    p->piece_rect[0].w = w_piece;
    p->piece_rect[0].h = h_piece;

    p->piece_rect[1].x = 930;
    p->piece_rect[1].y = 200 + h_piece + 30;
    p->piece_rect[1].w = w_piece;
    p->piece_rect[1].h = h_piece;

    p->piece_start[0] = p->piece_rect[0];
    p->piece_start[1] = p->piece_rect[1];

    p->dragging = 0;
    p->selected_piece = -1;
    p->offset_x = 0;
    p->offset_y = 0;
    p->termine = 0;
    p->perdu = 0;
    p->sound_played = 0;

    p->tick_channel = -1;
    p->tick_started = 0;

    p->timer_start = 0;
    p->timer_duration = 11000;

    p->show_message = 0;
    p->message[0] = '\0';

    mettre_a_jour_layout_puzzle(p, renderer);
}

void gerer_evenements_puzzle(puzel *p, SDL_Renderer *renderer, int *etat)
{
    SDL_Event e;
    int mx, my;
    int i, target_index, ok, sel;

    mettre_a_jour_layout_puzzle(p, renderer);

    lancer_tick_si_necessaire(p);
    verifier_fin_tick(p);

    SDL_GetMouseState(&mx, &my);
    p->hover_retour = point_dans_rect_puzzle(mx, my, p->btn_retour);
    p->hover_option = point_dans_rect_puzzle(mx, my, p->btn_option);

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            p->running = 0;

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            *etat = 0;

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            SDL_GetMouseState(&mx, &my);

            if (point_dans_rect_puzzle(mx, my, p->btn_retour))
            {
                if (p->tick_channel != -1)
                    Mix_HaltChannel(p->tick_channel);

                p->tick_channel = -1;
                p->tick_started = 0;
                *etat = ETAT_OPTION;
                return;
            }

            if (point_dans_rect_puzzle(mx, my, p->btn_option))
            {
                if (p->tick_channel != -1)
                    Mix_HaltChannel(p->tick_channel);

                p->tick_channel = -1;
                p->tick_started = 0;
                *etat = ETAT_OPTION;
                return;
            }

            if (p->perdu || p->termine)
                continue;

            for (i = 0; i < 2; i++)
            {
                if (p->side_piece_visible[i] &&
                    point_dans_rect_puzzle(mx, my, p->piece_rect[i]))
                {
                    p->dragging = 1;
                    p->selected_piece = i;
                    p->offset_x = mx - p->piece_rect[i].x;
                    p->offset_y = my - p->piece_rect[i].y;
                    break;
                }
            }
        }

        if (e.type == SDL_MOUSEMOTION &&
            p->dragging &&
            p->selected_piece != -1 &&
            !p->perdu &&
            !p->termine)
        {
            p->piece_rect[p->selected_piece].x = e.motion.x - p->offset_x;
            p->piece_rect[p->selected_piece].y = e.motion.y - p->offset_y;
        }

        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
        {
            if (p->dragging &&
                p->selected_piece != -1 &&
                !p->perdu &&
                !p->termine)
            {
                sel = p->selected_piece;
                ok = 0;
                target_index = p->side_piece_index[sel];

                {
                    int cx = p->piece_rect[sel].x + p->piece_rect[sel].w / 2;
                    int cy = p->piece_rect[sel].y + p->piece_rect[sel].h / 2;

                    if (!p->case_remplie[target_index] &&
                        point_dans_rect_puzzle(cx, cy, p->dest[target_index]))
                    {
                        p->piece_rect[sel] = p->dest[target_index];
                        p->case_remplie[target_index] = 1;
                        p->side_piece_visible[sel] = 0;
                        p->feedback_piece[sel] = 1;
                        p->feedback_timer[sel] = SDL_GetTicks();
                        ok = 1;
                    }
                }

                if (!ok)
                {
                    p->feedback_piece[sel] = 2;
                    p->feedback_timer[sel] = SDL_GetTicks();
                    p->piece_rect[sel] = p->piece_start[sel];
                }

                p->dragging = 0;
                p->selected_piece = -1;
            }
        }
    }

    verifier_fin_tick(p);

    if (!p->side_piece_visible[0] && !p->side_piece_visible[1] && !p->perdu)
    {
        p->termine = 1;

        if (p->tick_channel != -1)
            Mix_HaltChannel(p->tick_channel);

        p->tick_channel = -1;
        p->tick_started = 0;

        if (p->sound_win && !p->sound_played)
        {
            Mix_PlayChannel(-1, p->sound_win, 0);
            p->sound_played = 1;
        }

        set_message(p, "Bravo !", (SDL_Color){0, 220, 0, 255});
    }
}

void afficher_puzzle(puzel *p, SDL_Renderer *renderer)
{
    int i;
    Uint32 now = SDL_GetTicks();
    SDL_Rect bg = {0, 0, 1200, 800};

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, p->background, NULL, &bg);

    afficher_timer_animation(renderer, p);

    if (p->image)
    {
        for (i = 0; i < 9; i++)
        {
            if (p->case_remplie[i])
            {
                SDL_RenderCopy(renderer, p->image, &p->src[i], &p->dest[i]);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 40, 25, 10, 200);
                SDL_RenderFillRect(renderer, &p->dest[i]);
                SDL_SetRenderDrawColor(renderer, 120, 80, 40, 255);
                SDL_RenderDrawRect(renderer, &p->dest[i]);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        for (i = 0; i < 9; i++)
            SDL_RenderDrawRect(renderer, &p->dest[i]);

        for (i = 0; i < 2; i++)
        {
            int show_feedback = (p->feedback_piece[i] != 0 &&
                                 (now - p->feedback_timer[i]) < FEEDBACK_MS);

            if (p->side_piece_visible[i] || show_feedback)
            {
                int idx = p->side_piece_index[i];
                SDL_RenderCopy(renderer, p->image, &p->src[idx], &p->piece_rect[i]);

                if (show_feedback)
                {
                    if (p->feedback_piece[i] == 1)
                        SDL_SetRenderDrawColor(renderer, 0, 220, 0, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, 220, 0, 0, 255);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
                }

                SDL_RenderDrawRect(renderer, &p->piece_rect[i]);

                {
                    SDL_Rect b2 = {
                        p->piece_rect[i].x - 1,
                        p->piece_rect[i].y - 1,
                        p->piece_rect[i].w + 2,
                        p->piece_rect[i].h + 2
                    };
                    SDL_RenderDrawRect(renderer, &b2);
                }
            }

            if (p->feedback_piece[i] != 0 &&
                (now - p->feedback_timer[i]) >= FEEDBACK_MS)
                p->feedback_piece[i] = 0;
        }
    }

    dessiner_bouton_image(renderer, p->btn_option_img, p->btn_option, p->hover_option);
    dessiner_bouton_image(renderer, p->btn_retour_img, p->btn_retour, p->hover_retour);

    if (p->termine)
    {
        SDL_SetRenderDrawColor(renderer, 0, 220, 0, 255);

        {
            SDL_Rect border = {
                p->zone_puzzle.x - 4,
                p->zone_puzzle.y - 4,
                p->zone_puzzle.w + 8,
                p->zone_puzzle.h + 8
            };

            SDL_RenderDrawRect(renderer, &border);

            border.x -= 2;
            border.y -= 2;
            border.w += 4;
            border.h += 4;

            SDL_RenderDrawRect(renderer, &border);
        }
    }

    if (p->perdu)
    {
        SDL_SetRenderDrawColor(renderer, 220, 0, 0, 255);

        {
            SDL_Rect border = {
                p->zone_puzzle.x - 4,
                p->zone_puzzle.y - 4,
                p->zone_puzzle.w + 8,
                p->zone_puzzle.h + 8
            };

            SDL_RenderDrawRect(renderer, &border);

            border.x -= 2;
            border.y -= 2;
            border.w += 4;
            border.h += 4;

            SDL_RenderDrawRect(renderer, &border);
        }
    }

    if (p->show_message)
    {
        if ((now - p->message_timer) < MESSAGE_MS)
        {
            SDL_Rect msg_bg = {380, 70, 440, 70};

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
            SDL_RenderFillRect(renderer, &msg_bg);

            SDL_SetRenderDrawColor(renderer,
                                   p->message_color.r,
                                   p->message_color.g,
                                   p->message_color.b,
                                   255);
            SDL_RenderDrawRect(renderer, &msg_bg);

            {
                SDL_Rect msg_bg2 = {378, 68, 444, 74};
                SDL_RenderDrawRect(renderer, &msg_bg2);
            }

            if (p->font)
            {
                int tw, th;

                if (TTF_SizeUTF8(p->font, p->message, &tw, &th) == 0)
                {
                    int tx = msg_bg.x + (msg_bg.w - tw) / 2;
                    int ty = msg_bg.y + (msg_bg.h - th) / 2;

                    afficher_texte(renderer, p->font, p->message, p->message_color, tx, ty);
                }
            }
        }
        else
        {
            p->show_message = 0;
        }
    }

    SDL_RenderPresent(renderer);
}

void nettoyer_puzzle(puzel *p)
{
    if (p->tick_channel != -1)
        Mix_HaltChannel(p->tick_channel);

    if (p->image) SDL_DestroyTexture(p->image);
    if (p->background) SDL_DestroyTexture(p->background);
    if (p->sound_win) Mix_FreeChunk(p->sound_win);
    if (p->tick_sound) Mix_FreeChunk(p->tick_sound);
    if (p->btn_retour_img) SDL_DestroyTexture(p->btn_retour_img);
    if (p->btn_option_img) SDL_DestroyTexture(p->btn_option_img);
    if (p->font) TTF_CloseFont(p->font);

    TTF_Quit();
    Mix_CloseAudio();
}
