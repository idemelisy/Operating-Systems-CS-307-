#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello world (pid:%d)\n", (int) getpid());
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } 
    else if (rc == 0) {
        // child (new process)
        printf("hello, I am child (pid:%d)\n", (int) getpid());
    } 
    else {
        // parent goes down this path (original process)
        printf("hello, I am parent of %d (pid:%d)\n", rc, (int) getpid());
        if (fork()) {
            // parent continues from here, but we miss the ID of the new child
            //sleep(0.1);
            printf("I am still the parent with a new child (pid:%d)\n", (int) getpid());
        } else { 
            // The new child goes from here!
            printf("hello, I am another child (pid:%d)\n", (int) getpid());
        } // We miss the fork failure in this case!
        sleep(2);
    }
    printf("Everybody prints this at the end (pid:%d)\n",(int) getpid());
    
    return 0;
}
