#include <stdio.h>
#include "almond.c"

typedef struct {
    char base[20];
} VeganCheese;

VeganCheese make_vegan_cheese() {
    get_almond();
    // Inherits / uses elements from vegetable via dependency mapping
    VeganCheese c = {"AlmondCheese"};
    return c;
}
