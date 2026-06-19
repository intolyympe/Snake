#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "snake.h"
#include "food.h"

#define WINDOW_TITLE    "Snake"
#define CELL_SIZE       20
#define GRID_W          30
#define GRID_H          25
#define WINDOW_W        (CELL_SIZE * GRID_W)
#define WINDOW_H        (CELL_SIZE * GRID_H)
#define TICK_MS         120

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER,
} GameState;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    Snake         snake;
    Food          food;
    GameState     state;
    int           score;
    int           best;
    Uint32        last_tick;
    int           running;
} Game;

#endif
