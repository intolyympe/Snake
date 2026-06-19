#include "snake.h"
#include <string.h>

void snake_init(Snake *s, int grid_w, int grid_h)
{
    memset(s, 0, sizeof(*s));
    s->length = 3;
    s->dir = DIR_RIGHT;
    s->next_dir = DIR_RIGHT;
    s->alive = 1;

    int sx = grid_w / 2;
    int sy = grid_h / 2;
    for (int i = 0; i < s->length; i++)
    {
        s->body[i].x = sx - i;
        s->body[i].y = sy;
    }
}

void snake_set_dir(Snake *s, Direction d)
{
    if (d == DIR_UP && s->dir == DIR_DOWN)
        return;
    if (d == DIR_DOWN && s->dir == DIR_UP)
        return;
    if (d == DIR_LEFT && s->dir == DIR_RIGHT)
        return;
    if (d == DIR_RIGHT && s->dir == DIR_LEFT)
        return;
    s->next_dir = d;
}

int snake_move(Snake *s, int grid_w, int grid_h)
{
    s->dir = s->next_dir;

    for (int i = s->length - 1; i > 0; i--)
        s->body[i] = s->body[i - 1];

    switch (s->dir)
    {
    case DIR_UP:
        s->body[0].y--;
        break;
    case DIR_DOWN:
        s->body[0].y++;
        break;
    case DIR_LEFT:
        s->body[0].x--;
        break;
    case DIR_RIGHT:
        s->body[0].x++;
        break;
    }

    if (s->body[0].x < 0 || s->body[0].x >= grid_w ||
        s->body[0].y < 0 || s->body[0].y >= grid_h)
        return 1;

    for (int i = 1; i < s->length - 1; i++)
    {
        if (s->body[0].x == s->body[i].x &&
            s->body[0].y == s->body[i].y)
            return 1;
    }

    return 0;
}

int snake_eats(const Snake *s, int fx, int fy)
{
    return (s->body[0].x == fx && s->body[0].y == fy);
}

int snake_occupies(const Snake *s, int x, int y)
{
    for (int i = 0; i < s->length; i++)
        if (s->body[i].x == x && s->body[i].y == y)
            return 1;
    return 0;
}
