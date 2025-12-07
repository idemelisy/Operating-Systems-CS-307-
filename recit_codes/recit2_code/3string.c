#include <stdio.h>
#include <string.h>
#include  <ctype.h>

int main() {
    // String Declaration and Initialization
    char str1[50] = "Hello";
    char str2[50] = " World!";
    char str3[50];


    // String Length
    printf("Length of String: %ld\n", strlen(str1));

    // String Concatenation
    strcat(str1, str2);
    printf("Concatenated String: %s\n", str1);

    strcpy(str3, str1);
    printf("Copied String: %s\n", str3);

    // String Comparison
    if (strcmp(str1, str3) == 0) {
        printf("Strings are equal.\n");
    } else {
        printf("Strings are not equal.\n");
    }

    // Finding a Substring
    char str4 [100] = "lorem ipsum dolor sit amet";
    char *substr = strstr(str4, "sit");

    // printf("returned value %s\n",substr);
    // printf("substr with s %s\n ", substr);
    // printf("substr with p %p\n", substr);
    // printf("str4 with s %s\n ", str4);
    // printf("str4 with p %p\n", str4);
    printf("str4 with c %c\n", *str4);

    if (substr) {
        printf("Substring found at position: %ld\n", substr - str4);
    } else {
        printf("Substring not found.\n");
    }

    // Character Search
    char *ch = strchr(str4, 'l');

    if (ch) {
        printf("First occurrence of the char: %ld\n", ch - str1);
    } else {
        printf("Character not found.\n");
    }


    // challenge: try to implement trim function.
    // trim function removes all leading and trailing whitespace characters 
    // (spaces, tabs, newlines) from a given string.
    // " \t  Hello World!   \n" -> "Hello World!"
    // "      " -> ""
    // "cs307" -> "cs307"
    char *trim(char *str){
        // hint1: isspace() function checks for any whitespace (space, \t, \n, etc.). 
        // (include <ctype.h>)

        // hint2: 

        // initially
        // "  \n Hello World!      \t  \0" 
        //  ^                         ^
        // str                       end <- (str + strlen(str)-1)
         
        // after trim
        // "  \n Hello World!\0      \t  " 
        //       ^          ^
        //      str        end
        
    
        // your code here

        return str;
    }
    // char example[100] = " \t  Hello World!   \n";
    // printf("before trim |%s| \n", example);
    // printf("after trim |%s| \n" , trim(example));



    
    printf("strtok function:\n");

    char text[] = "apple,banana,cherry,grape";
    const char *delimiter = ",";
    char *token = strtok(text, delimiter);
    
    while (token != NULL) {
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiter);
    }


    return 0;
}
