#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Couleurs */
#define COL_BG 0x0f, 0x0f, 0x1a, 0xff
#define COL_GRID 0x1a, 0x1a, 0x2e, 0xff
#define COL_HEAD 0x00, 0xe6, 0x76, 0xff
#define COL_BODY 0x00, 0x99, 0x4d, 0xff
#define COL_FOOD 0xff, 0x4d, 0x4d, 0xff
#define COL_OVERLAY 0x00, 0x00, 0x00, 0xb0

static void set_color(SDL_Renderer *r, Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    SDL_SetRenderDrawColor(r, R, G, B, A);
}

int game_init(Game *g)
{
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
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

    g->state = STATE_MENU;
    g->score = 0;
    g->best = 0;
    g->running = 1;
    return 1;
}

void game_quit(Game *g)
{
    if (g->renderer)
        SDL_DestroyRenderer(g->renderer);
    if (g->window)
        SDL_DestroyWindow(g->window);
    SDL_Quit();
}

static void draw_cell(SDL_Renderer *r, int x, int y)
{
    SDL_Rect rect = {
        x * CELL_SIZE + 1,
        y * CELL_SIZE + 1,
        CELL_SIZE - 2,
        CELL_SIZE - 2};
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

static void update_title(Game *g)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "Snake — Score: %d  Best: %d", g->score, g->best);
    SDL_SetWindowTitle(g->window, buf);
}

static void render_playing(Game *g)
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

    SDL_RenderPresent(r);
}

static void render_overlay(Game *g, const char *msg)
{
    SDL_Renderer *r = g->renderer;

    set_color(r, COL_OVERLAY);
    SDL_Rect full = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderFillRect(r, &full);

    char buf[128];
    snprintf(buf, sizeof(buf), "%s  |  Score: %d  Best: %d", msg, g->score, g->best);
    SDL_SetWindowTitle(g->window, buf);

    SDL_RenderPresent(r);
}

static void start_game(Game *g)
{
    snake_init(&g->snake, GRID_W, GRID_H);
    food_spawn(&g->food, &g->snake, GRID_W, GRID_H);
    g->score = 0;
    g->last_tick = SDL_GetTicks();
    g->state = STATE_PLAYING;
    update_title(g);
}

void game_run(Game *g)
{
    render_overlay(g, "SNAKE — Appuie sur ENTREE");

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
            Uint32 now = SDL_GetTicks();
            Uint32 speed = (Uint32)(TICK_MS - (g->score * 2));
            if (speed < 50)
                speed = 50;

            if (now - g->last_tick >= speed)
            {
                g->last_tick = now;

                int dead = snake_move(&g->snake, GRID_W, GRID_H);
                if (dead)
                {
                    if (g->score > g->best)
                        g->best = g->score;
                    g->state = STATE_GAME_OVER;
                    render_playing(g);
                    render_overlay(g, "GAME OVER — ENTREE pour rejouer");
                }
                else
                {
                    if (snake_eats(&g->snake, g->food.x, g->food.y))
                    {
                        g->score++;
                        if (g->snake.length < SNAKE_MAX)
                            g->snake.length++;
                        food_spawn(&g->food, &g->snake, GRID_W, GRID_H);
                        update_title(g);
                    }
                    render_playing(g);
                }
            }
            else
            {
                SDL_Delay(1);
            }
        }
    }
}
