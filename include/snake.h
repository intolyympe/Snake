#ifndef SNAKE_H
#define SNAKE_H

#define SNAKE_MAX 750   /* GRID_W * GRID_H */

typedef struct {
    int x, y;
} Vec2;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
} Direction;

typedef struct {
    Vec2      body[SNAKE_MAX];
    int       length;
    Direction dir;
    Direction next_dir;
    int       alive;
} Snake;

void snake_init(Snake *s, int grid_w, int grid_h);
void snake_set_dir(Snake *s, Direction d);
int  snake_move(Snake *s, int grid_w, int grid_h);
int  snake_eats(const Snake *s, int fx, int fy);
int  snake_occupies(const Snake *s, int x, int y);

#endif
