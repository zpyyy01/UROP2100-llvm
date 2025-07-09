#include <stdio.h>

int main() {
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0)
            printf("Hello\n");
        else
            printf("World\n");

        printf("In loop: %d\n", i + 1);
    }
    printf("Done\n");
    return 0;
}