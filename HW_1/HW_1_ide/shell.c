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
            freeCompiledCmd(&command);
            continue;
        }

        if (command.isQuit == 1) {
            printf("Exiting shell...\n");
            freeCompiledCmd(&command);
            break;
        }
         if(result !=0 && command.isQuit!=1){
            if(command.inLoop.n==0){
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
            }   }
            else{//LOOpipe
                int num_children=(int)(command.before.n + (command.inLoop.n * command.loopLen) + command.after.n);
                pid_t pids[num_children];
                int child_index=0;
                int input_fd=STDIN_FILENO;
                //before part
                for (int i = 0; i < (int)command.before.n; i++) {
                    int p[2];
                    if (pipe(p) == -1) { printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1); }
                    pid_t pid=fork();
                    if(pid<0){
                        printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1);
                    }
                    
                    //----child
                    if(pid==0){
                        // Input: First child checks command.inFile, others read from input_fd
                        if(i>0){
                            dup2(input_fd, STDIN_FILENO);
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
                        
                        // Output: Last child must feed the loop
                        if (i < (int)command.before.n - 1) {//inner commands
                            dup2(p[1], STDOUT_FILENO);
                        } else {
                            // Last command in before stage feeds the loop
                            dup2(p[1], STDOUT_FILENO);
                        }
                        
                        close(p[0]);
                        close(p[1]);
                        execvp(command.before.argvs[i][0],command.before.argvs[i]);
                        printf("execvp failed\n");
                        exit(1);
                    }
                    else if(pid>0){
                        //parentlogic
                        pids[child_index ++]=pid;
                        close(p[1]);
                        if (i > 0 && input_fd != STDIN_FILENO) {
                            close(input_fd);
                        }
                        input_fd=p[0];
                    }
                }//in loop part
                for (int k = 0; k < (int)command.loopLen; k++){
                    for(int i=0;i<(int)command.inLoop.n;i++){
                    int p[2];
                    if (pipe(p) == -1) { printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1); }
                    pid_t pid=fork();
                    if(pid<0){
                        printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1);
                    }
                    //----child
                    if(pid==0){
                        // Input: All children read from input_fd (baton from previous stage)
                        if(i>0){
                            dup2(input_fd, STDIN_FILENO);
                            close(input_fd);
                        } else {
                            // This is the FIRST command of this pipeline (i == 0)
                            
                            // FIX: Check if we are the VERY first stage
                            if (command.before.n == 0 && k == 0 && command.inFile != NULL) {
                                // This is the first command of the whole command, handle inFile
                                int fd_in = open(command.inFile, O_RDONLY);
                                if(fd_in < 0){
                                    printf("error opening input file\n");
                                    exit(1);
                                }
                                dup2(fd_in, STDIN_FILENO);
                                close(fd_in);
                            } else {
                                // Not the very first command, read from the "baton" (input_fd)
                                dup2(input_fd, STDIN_FILENO);
                                close(input_fd);
                            }
                        }
                        
                        // Output: Last child feeds next iteration or Stage 3
                        if (i < (int)command.inLoop.n - 1) {//inner commands
                            dup2(p[1], STDOUT_FILENO);
                        } else {
                            // This is the LAST command of this pipeline (i == n-1)

                            // FIX: Check if we are the VERY last stage
                            if (k == (int)command.loopLen - 1 && command.after.n == 0) {
                                // This is the last iteration AND there is no 'after' stage
                                // Handle final output just like your simple pipeline
                                if (command.outFile != NULL) {
                                    int fd_out = open(command.outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                    if(fd_out < 0){
                                        printf("error opening output file\n");
                                        exit(1);
                                    }
                                    dup2(fd_out, STDOUT_FILENO);
                                    close(fd_out);
                                }
                                // If command.outFile is NULL, output goes to STDOUT (which is correct)
                            } else {
                                // This is not the last iteration, OR there is an 'after' stage
                                // Feed the next pipe
                                dup2(p[1], STDOUT_FILENO);
                            }
                        }
                        
                        close(p[0]);
                        close(p[1]);
                        execvp(command.inLoop.argvs[i][0],command.inLoop.argvs[i]);
                        printf("execvp failed\n");
                        exit(1);
                    }
                    else if(pid>0){
                        //parentlogic
                        pids[child_index ++]=pid;
                        close(p[1]);
                        if (i > 0 && input_fd != STDIN_FILENO) {
                            close(input_fd);
                        }
                        input_fd=p[0];
                    }
                }}//after part
                for(int i=0;i<(int)command.after.n;i++){
                    int p[2];
                    if (pipe(p) == -1) { printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1); }
                    pid_t pid=fork();
                    if(pid<0){
                        printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1);
                    }

                    //----child
                    if(pid==0){
                        // Input: All children read from input_fd (baton from previous stage)
                        if(i>0){
                            dup2(input_fd, STDIN_FILENO);
                            close(input_fd);
                        } else {
                            // First child also reads from input_fd
                            dup2(input_fd, STDIN_FILENO);
                            close(input_fd);
                        }
                        
                        // Output: Keep simple pipeline logic - check for command.outFile on last command
                        if (i < (int)command.after.n - 1) {//inner commands
                            dup2(p[1], STDOUT_FILENO);
                        } else if (i == (int)command.after.n - 1 && command.outFile != NULL) {
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
                        execvp(command.after.argvs[i][0],command.after.argvs[i]);
                        printf("execvp failed\n");
                        exit(1);
                    }
                    else if(pid>0){
                        //parentlogic
                        pids[child_index ++]=pid;
                        close(p[1]);
                        if (i > 0 && input_fd != STDIN_FILENO) { // <-- ADD THIS
                            close(input_fd);
                        }
                        input_fd=p[0];
                    }
                       
                }if(input_fd != STDIN_FILENO) {
                    close(input_fd);
                }
                //wait part
                for(int i=0;i<num_children;i++){
                    waitpid(pids[i], NULL, 0);
                }
            }
            freeCompiledCmd(&command);   }//end of while free 
    }
    
    free(line);
    freeParser(&parser);
    return 0;

}

