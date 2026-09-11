#ifndef ALFREDO_PASTA_H
#define ALFREDO_PASTA_H

#include "pasta.c"
#include "vegan_cheese.c"
#include "avocado_oil.c"
#include "mushroom.c"

typedef struct {
    Pasta base_pasta;
    VeganCheese cheese;
    char secret_touch[30];
} AlfredoPasta;

#endif
