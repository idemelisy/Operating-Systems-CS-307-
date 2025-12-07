#include <stdio.h>

void scanf_demo() {
    
    double num1, num2;

    printf("Enter a number: ");
    scanf("%lf", &num1);

    printf("Enter another number: ");
    scanf("%lf", &num2);

    printf("num1 = %lf\n", num1);
    printf("num2 = %lf\n", num2);
}

int main() {
    scanf_demo();
    return 0;
}
