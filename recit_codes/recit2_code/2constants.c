#include <stdio.h>

// Global constants
#define PI 3.14159
#define MAX_VALUE 100

const int BIRTH_YEAR = 2001;  // Constant integer
const char INITIAL = 'A';  // Constant character

int main() {
    // Local constant
    const double GRAVITY = 9.81;

    // Printing constants
    printf("Global Constants:\n");
    printf("PI: %.5f\n", PI);
    printf("MAX_VALUE: %d\n", MAX_VALUE);
    printf("BIRTH_YEAR: %d\n", BIRTH_YEAR);
    printf("INITIAL: %c\n", INITIAL);

    printf("\nLocal Constant:\n");
    printf("GRAVITY: %.2lf m/s^2\n", GRAVITY);

    return 0;
}
