#ifndef FOOD_H
#define FOOD_H

#include "snake.h"

typedef struct
{
    int x, y;
} Food;

void food_spawn(Food *f, const Snake *s, int grid_w, int grid_h);

#endif
