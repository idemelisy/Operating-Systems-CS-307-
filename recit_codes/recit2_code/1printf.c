#include <stdio.h>
#include <string.h>

void printf_demo(){

    printf("CS307 - Operating Systems\n");

    // Integers
    int num1 = 10;
    int num2 = 20;

    printf("Integer1: %d \nInteger2: %d\n",num1, num2);

    // Double & Float
    float num3 = 13.5;
    double num4 = 3.14159265359;

    printf("Float1: %f\nDouble1: %.11lf\n", num3, num4);

    // Characters
    char chr = 'a';
    printf("Character: %c\n", chr);

    // Strings
    char str[10] = "Hello";
    printf("String: %s\n", str);


    printf("Length of String: %ld\n", strlen(str));  
    printf("size of string %ld\n" , sizeof(str)); 
    printf("char 8 %c\n" , str[8]); 
}

int main() {
    printf_demo();
    return 0;
}