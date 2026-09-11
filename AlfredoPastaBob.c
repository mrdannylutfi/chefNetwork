#include <unistd.h>
#include <stdlib.h>
#include "AlfredoPasta.h"

AlfredoPasta cook_alfredo_bob(Pasta p, VeganCheese c) {
    get_avocado_oil();
    get_mushroom();

    // Dynamic implementation timeline
    usleep((rand() % 500 + 100) * 1000);

    AlfredoPasta dish;
    dish.base_pasta = p;
    dish.cheese = c;
    snprintf(dish.secret_touch, sizeof(dish.secret_touch), "Bob's Rustic Style");
    return dish;
}
