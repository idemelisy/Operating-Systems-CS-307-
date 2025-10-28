//İde Melis Yılmaz CS 307 Programming Assignment 1
#include "parser.h" 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void) {
    sparser_t parser;
    if (!initParser(&parser)) {
        fprintf(stderr, "Failed to initialize parser\n");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("SUShell$ ");
        fflush(stdout);

        ssize_t length = getline(&line, &len, stdin);
        if (length == -1) { //Ctrl+D (End-of-File)
            break;
        }

        compiledCmd command;
        int result = compileCommand(&parser, line, &command);
        if (result==0) {
            printf("Parser ERROR\n");
            continue;
        }

        if (command.isQuit == 1) {
            printf("Exiting shell...\n");
            freeCompiledCmd(&command);
            break;
        }
         if(result !=0 && command.isQuit!=1){
            //printf("main logic here"); //Delete later
            // printCompiledCmd(&command);
            int input_fd=STDIN_FILENO;
            int num_commands= command.before.n;
            pid_t pids[num_commands];
            for(int i=0;i<num_commands;i++){
                int p[2];
                if (pipe(p) == -1) {
                    printf("pipe failed\n");
                    // parent: close input_fd if it was a pipe from previous step
                    if (input_fd != STDIN_FILENO) close(input_fd);
                    exit(1);
                }
                pids[i]=fork();
                if(pids[i]<0){
                    printf("fork failed\n");
                    close(p[0]);
                    close(p[1]);
                    if (input_fd != STDIN_FILENO) {
                        close(input_fd);
                    }
                    exit(1);
        }
                 if(pids[i]==0){//child
                    if(i>0){
                        dup2(input_fd, STDIN_FILENO) ;
                        close(input_fd);
                    }else if (i == 0 && command.inFile != NULL) {//first command
                        int fd_in=open(command.inFile,O_RDONLY);
                        if(fd_in<0){
                            printf("error opening input file\n");
                            exit(1);
                        }
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                    }
                    if (i < num_commands - 1) {//inner commands
                        dup2(p[1], STDOUT_FILENO);
                    } else if (i == num_commands - 1 && command.outFile != NULL) {
                            int fd_out=open(command.outFile,O_WRONLY|O_CREAT|O_TRUNC,0644);
                            if(fd_out<0){
                                printf("error opening output file\n");
                                exit(1);
                            }
                            dup2(fd_out, STDOUT_FILENO);
                            close(fd_out);
                        }
                    close(p[0]);
                    close(p[1]);
                    execvp(command.before.argvs[i][0],command.before.argvs[i]);
                    printf("execvp failed\n");
                    exit(1);
                }
                else if(pids[i]>0){
                    close(p[1]);
                    if (i > 0 && input_fd != STDIN_FILENO) {
                        close(input_fd);
                    } 
                    input_fd = p[0];
                }
                
            }
            if (input_fd != STDIN_FILENO) close(input_fd);
            for(int i=0;i<num_commands;i++){
                wait(NULL); //Wait until child finishes
            }    
        }
    }
    
    free(line);
    freeParser(&parser);
    return 0;
}

