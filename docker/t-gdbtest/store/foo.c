#include <stdio.h>

int deep(int k);

int foo(int i) {
    int k = deep(i);
    printf("foo: i=%d => r=%d\n", i, i + k);
    return i + k;
}
