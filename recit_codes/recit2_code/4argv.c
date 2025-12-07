#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Check if correct number of arguments are provided
    if (argc != 4) {
        printf("Usage: %s <name> <age> <height>\n", argv[0]);
        return 1;
    }

    // Display command-line arguments
    printf("Name: %s\n", argv[1]);
    printf("Age: %d\n", atoi(argv[2]));  // Convert string to integer
    printf("Height: %.2f cm\n", atof(argv[3]));  // Convert string to float

    return 0;
}
