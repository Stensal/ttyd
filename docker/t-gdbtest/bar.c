#include <stdio.h>

int bar(int j) {
    printf("bar: j=%d => r=%d\n", j, j * 3);
    return j * 3;
}
