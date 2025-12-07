#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("hello world (pid:%d)\n", (int) getpid());
    int count = 0;
    while (count < 3) {
        int rc = fork();
		if (rc > 0) {
			printf("I am parent of %d (pid:%d)\n", rc, (int) getpid());
		} else {
			// child
			if (count <3) {
			   count++; // What if I replace this line with count--;
			}
		}
		count++;
    }

	sleep(1);
    return 0;
}
