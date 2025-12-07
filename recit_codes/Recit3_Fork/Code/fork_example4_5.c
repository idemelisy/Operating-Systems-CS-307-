#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello world (pid:%d)\n", (int) getpid());
    int rc1 = fork();
    
    if (rc1 < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc1 == 0) {
        // child (new process)
        printf("hello, I am child (pid:%d)\n", (int) getpid());
    } else {
        // parent goes down this path (original process)
        //printf("hello, I am parent of %d (pid:%d)\n", rc1, (int) getpid());

        int rc2 = fork();

        if (rc2 < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc2 == 0) {
            // child (new process)
            printf("hello, I am child (pid:%d)\n", (int) getpid());
        } else {
            // parent goes down this path (original process)
            // wait(NULL);

            printf("hello, I am parent of %d (pid:%d)\n", rc2, (int) getpid());
        }
        
        sleep(1);
    }
    
    
    return 0;
}
