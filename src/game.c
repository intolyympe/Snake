#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COL_BG      0x0f, 0x0f, 0x1a, 0xff
#define COL_GRID    0x1a, 0x1a, 0x2e, 0xff
#define COL_HEAD    0x00, 0xe6, 0x76, 0xff
#define COL_BODY    0x00, 0x99, 0x4d, 0xff
#define COL_FOOD    0xff, 0x4d, 0x4d, 0xff
#define COL_OVERLAY 0x00, 0x00, 0x00, 0xb0

static void set_color(SDL_Renderer *r, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    SDL_SetRenderDrawColor(r, R, G, B, A);
}

/* ── Text ──────────────────────────────────────────────────────────── */

static void draw_text(SDL_Renderer *r, TTF_Font *font,
                      const char *text, int cx, int y, SDL_Color col)
{
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, col);
    if (!surf)
        return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if (!tex)
        return;
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect dst = {cx - w / 2, y, w, h};
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

/* ── Leaderboard ───────────────────────────────────────────────────── */

static void leaderboard_insert(Game *g, int score)
{
    for (int i = 0; i < LEADERBOARD_SIZE; i++)
    {
        if (score > g->scores[i])
        {
            for (int j = LEADERBOARD_SIZE - 1; j > i; j--)
                g->scores[j] = g->scores[j - 1];
            g->scores[i] = score;
            return;
        }
    }
}

/* ── Init / Quit ───────────────────────────────────────────────────── */

int game_init(Game *g)
{
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() != 0)
    {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 0;
    }

    g->font_lg = TTF_OpenFont(FONT_PATH, 52);
    g->font_md = TTF_OpenFont(FONT_PATH, 26);
    g->font_sm = TTF_OpenFont(FONT_PATH, 18);
    if (!g->font_lg || !g->font_md || !g->font_sm)
    {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        return 0;
    }

    g->window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, 0);
    if (!g->window)
    {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 0;
    }

    g->renderer = SDL_CreateRenderer(
        g->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer)
    {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 0;
    }

    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);

    g->state   = STATE_MENU;
    g->score   = 0;
    g->running = 1;
    memset(g->scores, 0, sizeof(g->scores));
    return 1;
}

void game_quit(Game *g)
{
    if (g->font_sm) TTF_CloseFont(g->font_sm);
    if (g->font_md) TTF_CloseFont(g->font_md);
    if (g->font_lg) TTF_CloseFont(g->font_lg);
    if (g->renderer) SDL_DestroyRenderer(g->renderer);
    if (g->window)   SDL_DestroyWindow(g->window);
    TTF_Quit();
    SDL_Quit();
}

/* ── Drawing helpers ───────────────────────────────────────────────── */

static void draw_cell(SDL_Renderer *r, int x, int y)
{
    SDL_Rect rect = {x * CELL_SIZE + 1, y * CELL_SIZE + 1,
                     CELL_SIZE - 2,     CELL_SIZE - 2};
    SDL_RenderFillRect(r, &rect);
}

static void draw_grid(SDL_Renderer *r)
{
    set_color(r, COL_GRID);
    for (int x = 0; x <= GRID_W; x++)
        SDL_RenderDrawLine(r, x * CELL_SIZE, 0, x * CELL_SIZE, WINDOW_H);
    for (int y = 0; y <= GRID_H; y++)
        SDL_RenderDrawLine(r, 0, y * CELL_SIZE, WINDOW_W, y * CELL_SIZE);
}

static void render_field(Game *g)
{
    SDL_Renderer *r = g->renderer;

    set_color(r, COL_BG);
    SDL_RenderClear(r);
    draw_grid(r);

    set_color(r, COL_BODY);
    for (int i = 1; i < g->snake.length; i++)
        draw_cell(r, g->snake.body[i].x, g->snake.body[i].y);

    set_color(r, COL_HEAD);
    draw_cell(r, g->snake.body[0].x, g->snake.body[0].y);

    set_color(r, COL_FOOD);
    draw_cell(r, g->food.x, g->food.y);
}

/* ── Screens ───────────────────────────────────────────────────────── */

static void draw_leaderboard(Game *g, int start_y)
{
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color grey  = {160, 160, 160, 255};
    int cx = WINDOW_W / 2;

    draw_text(g->renderer, g->font_sm, "TOP SCORES", cx, start_y, grey);

    for (int i = 0; i < LEADERBOARD_SIZE; i++)
    {
        char line[32];
        if (g->scores[i] > 0)
            snprintf(line, sizeof(line), "#%d   %d", i + 1, g->scores[i]);
        else
            snprintf(line, sizeof(line), "#%d   ---", i + 1);
        draw_text(g->renderer, g->font_sm, line, cx, start_y + 30 + i * 26, white);
    }
}

static void render_menu(Game *g)
{
    SDL_Renderer *r = g->renderer;
    int cx = WINDOW_W / 2;

    SDL_Color green = {0x00, 0xe6, 0x76, 255};
    SDL_Color white = {255,  255,  255,  255};
    SDL_Color grey  = {160,  160,  160,  255};

    set_color(r, COL_BG);
    SDL_RenderClear(r);
    draw_grid(r);

    draw_text(r, g->font_lg, "SNAKE",                     cx, 55,  green);
    draw_text(r, g->font_md, "Press ENTER to play",       cx, 155, white);
    draw_text(r, g->font_sm, "Arrows  /  ZQSD  /  WASD", cx, 203, grey);

    SDL_SetRenderDrawColor(r, 50, 50, 70, 255);
    SDL_RenderDrawLine(r, cx - 110, 248, cx + 110, 248);

    draw_leaderboard(g, 262);

    SDL_RenderPresent(r);
}

static void render_game_over(Game *g)
{
    SDL_Renderer *r = g->renderer;
    int cx = WINDOW_W / 2;

    SDL_Color red   = {0xff, 0x4d, 0x4d, 255};
    SDL_Color white = {255,  255,  255,  255};
    SDL_Color grey  = {160,  160,  160,  255};

    render_field(g);

    set_color(r, COL_OVERLAY);
    SDL_Rect full = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderFillRect(r, &full);

    draw_text(r, g->font_lg, "GAME OVER",               cx, 45,  red);

    char buf[32];
    snprintf(buf, sizeof(buf), "Score : %d", g->score);
    draw_text(r, g->font_md, buf,                       cx, 145, white);

    SDL_SetRenderDrawColor(r, 50, 50, 70, 255);
    SDL_RenderDrawLine(r, cx - 110, 197, cx + 110, 197);

    draw_leaderboard(g, 212);

    draw_text(r, g->font_sm, "Press ENTER to play again", cx, 432, grey);

    SDL_RenderPresent(r);
}

/* ── Game logic ────────────────────────────────────────────────────── */

static void start_game(Game *g)
{
    snake_init(&g->snake, GRID_W, GRID_H);
    food_spawn(&g->food, &g->snake, GRID_W, GRID_H);
    g->score     = 0;
    g->last_tick = SDL_GetTicks();
    g->state     = STATE_PLAYING;
    SDL_SetWindowTitle(g->window, "Snake — Score: 0");
}

void game_run(Game *g)
{
    while (g->running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                g->running = 0;
                break;
            }

            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    g->running = 0;
                    break;

                case SDLK_RETURN:
                case SDLK_SPACE:
                    if (g->state != STATE_PLAYING)
                        start_game(g);
                    break;

                case SDLK_UP:
                case SDLK_z:
                case SDLK_w:
                    snake_set_dir(&g->snake, DIR_UP);
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    snake_set_dir(&g->snake, DIR_DOWN);
                    break;
                case SDLK_LEFT:
                case SDLK_q:
                case SDLK_a:
                    snake_set_dir(&g->snake, DIR_LEFT);
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    snake_set_dir(&g->snake, DIR_RIGHT);
                    break;
                default:
                    break;
                }
            }
        }

        if (g->state == STATE_PLAYING)
        {
            Uint32 now   = SDL_GetTicks();
            Uint32 speed = (Uint32)(TICK_MS - (g->score * 2));
            if (speed < 50)
                speed = 50;

            if (now - g->last_tick >= speed)
            {
                g->last_tick = now;

                int dead = snake_move(&g->snake, GRID_W, GRID_H);
                if (dead)
                {
                    leaderboard_insert(g, g->score);
                    g->state = STATE_GAME_OVER;
                    render_game_over(g);
                }
                else
                {
                    if (snake_eats(&g->snake, g->food.x, g->food.y))
                    {
                        g->score++;
                        if (g->snake.length < SNAKE_MAX)
                            g->snake.length++;
                        food_spawn(&g->food, &g->snake, GRID_W, GRID_H);

                        char buf[64];
                        snprintf(buf, sizeof(buf), "Snake — Score: %d  Best: %d",
                                 g->score, g->scores[0]);
                        SDL_SetWindowTitle(g->window, buf);
                    }
                    render_field(g);
                    SDL_RenderPresent(g->renderer);
                }
            }
            else
                SDL_Delay(1);
        }
        else
        {
            if (g->state == STATE_MENU)
                render_menu(g);
            else
                render_game_over(g);
            SDL_Delay(16);
        }
    }
}
