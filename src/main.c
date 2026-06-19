#include "game.h"
#include <stdlib.h>

int main(void)
{
    Game g = {0};

    if (!game_init(&g))
    {
        game_quit(&g);
        return EXIT_FAILURE;
    }

    game_run(&g);
    game_quit(&g);

    return EXIT_SUCCESS;
}
