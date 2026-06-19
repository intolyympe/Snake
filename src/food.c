#include "food.h"
#include <stdlib.h>

void food_spawn(Food *f, const Snake *s, int grid_w, int grid_h)
{
    int x, y;
    do {
        x = rand() % grid_w;
        y = rand() % grid_h;
    } while (snake_occupies(s, x, y));

    f->x = x;
    f->y = y;
}
