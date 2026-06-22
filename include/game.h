#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "snake.h"
#include "food.h"

#define WINDOW_TITLE "Snake"
#define CELL_SIZE 20
#define GRID_W 30
#define GRID_H 25
#define WINDOW_W (CELL_SIZE * GRID_W)
#define WINDOW_H (CELL_SIZE * GRID_H)
#define TICK_MS 120
#define LEADERBOARD_SIZE 5
#define FONT_PATH "assets/font.ttf"

typedef enum
{
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER,
} GameState;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font_lg;
    TTF_Font *font_md;
    TTF_Font *font_sm;
    Snake snake;
    Food food;
    GameState state;
    int score;
    int scores[LEADERBOARD_SIZE];
    Uint32 last_tick;
    int running;
} Game;

int game_init(Game *g);
void game_run(Game *g);
void game_quit(Game *g);

#endif
