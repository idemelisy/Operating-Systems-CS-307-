#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int count = 0;
    while (fork()){
        printf("Count: %d\n", count);
        if (count >= 10) {
            break;
        }
        count++;
    }
    
    return 0;
}
