#include <unistd.h>
#include <stdlib.h>
#include "AlfredoPasta.h"

AlfredoPasta cook_alfredo_alice(Pasta p, VeganCheese c) {
    get_avocado_oil();
    get_mushroom();

    // Unique randomized implementation time per turn
    usleep((rand() % 500 + 100) * 1000);

    AlfredoPasta dish;
    dish.base_pasta = p;
    dish.cheese = c;
    snprintf(dish.secret_touch, sizeof(dish.secret_touch), "Alice's Perfection");
    return dish;
}
