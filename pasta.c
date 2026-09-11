#include <stdio.h>
#include "vegetable.c"
#include "wheat.c"

typedef struct {
    char type[20];
} Pasta;

Pasta make_pasta() {
    get_wheat();
    get_vegetable();
    Pasta p = {"WheatPasta"};
    return p;
}
