#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int foo(int i);
int bar(int j);

int main(int argc, char* argv[]) {
    printf("argc=%d\n", argc);

    for (int i = 1; i < argc; i++) {
	int val = atoi(argv[i]);
        int a = foo(val);
        int b = bar(val);
        printf("r=%d\n", a + b);
        usleep(100000);
    }
    return 0;
}
