#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/wait.h>
#define MSGSIZE 16 
char* msg1 = "hello, world #1";
char* msg2 = "hello, world #2"; 
char* msg3 = "hello, world #3"; 

int main() 
{ 
	char inbuf[MSGSIZE*3]; 
	int fd[2], pid, nbytes; 

	if (pipe(fd) < 0) 
		exit(1); 

	if ((pid = fork()) > 0) { 
		// Removing below line will  block the program 
		close(fd[1]); 

		// read(fd[0], inbuf, MSGSIZE);
		// printf("%s from here\n", inbuf); 
		// read(fd[0], inbuf, MSGSIZE);
		// printf("%s from here\n", inbuf); 
		// read(fd[0], inbuf, MSGSIZE);
		// printf("%s from here\n", inbuf); 


		while ((nbytes = read(fd[0], inbuf, MSGSIZE)) > 0) 
			printf("%s from here\n", inbuf); 
		if (nbytes != 0) 
			exit(2); 
		wait(NULL);
		


	
		printf("Finished reading\n"); 
	} 

	else { 

		write(fd[1], msg1, MSGSIZE); 
		write(fd[1], msg2, MSGSIZE); 
		write(fd[1], msg3, MSGSIZE); 

		// If we remove belove line? 
		close(fd[1]); 
	} 
	return 0; 
} 
