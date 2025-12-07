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
        if(result !=0 && command.isQuit!=1){//no parser error and user wrote smth except :q
            if(command.inLoop.n==0){ //no loops ex: ls | wc
                int input_fd=STDIN_FILENO;//first command gets from keyboard
                int num_commands= command.before.n;// simple pipeline commands
                pid_t pids[num_commands];
                for(int i=0;i<num_commands;i++){
                    int p[2]; //array to read and write
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
                        if(i>0){//no the first command
                            dup2(input_fd, STDIN_FILENO) ;//read from input_fd
                            close(input_fd);
                        }
                        else if (i == 0 && command.inFile != NULL) {//first command and there is an input file
                            int fd_in=open(command.inFile,O_RDONLY);//read from input file
                            if(fd_in<0){
                                printf("error opening input file\n");
                                exit(1);
                            }
                            dup2(fd_in, STDIN_FILENO);//read from input file
                            close(fd_in);
                        }
                        if (i < num_commands - 1) {//inner commands
                            dup2(p[1], STDOUT_FILENO);//write to p[1]
                        } 
                        else if (i == num_commands - 1 && command.outFile != NULL) {//last command and there is an output file
                            int fd_out=open(command.outFile,O_WRONLY|O_CREAT|O_TRUNC,0644);//write to the output file
                            if(fd_out<0){
                                printf("error opening output file\n");
                                exit(1);
                            }
                            dup2(fd_out, STDOUT_FILENO);
                            close(fd_out);
                        }
                        close(p[0]);//read and write is done
                        close(p[1]);//read and write is done
                        execvp(command.before.argvs[i][0],command.before.argvs[i]);
                        printf("execvp failed\n");
                        exit(1);
                    }
                    else if(pids[i]>0){
                        close(p[1]);
                        if (i > 0 && input_fd != STDIN_FILENO) {
                            close(input_fd);
                        } 
                        input_fd = p[0];//parrent saves the input_fd for the next command
                    }
                    
                }//end of for
                if (input_fd != STDIN_FILENO){
                    close(input_fd);
                } 
                for(int i=0;i<num_commands;i++){
                    wait(NULL); //Wait until child finishes
                }   
            }//end of simple pipeline
            else{//LOOpipe if no loop else looppipe ex:echo 'a' | (rev)_2 | sort
                //same with simple pipe 
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
                        exit(1); 
                    }
                    pid_t pid=fork();
                    if(pid<0){
                        printf("fork failed\n");
                        close(p[0]);
                        close(p[1]);
                        exit(1);
                    }
                    
                    //----child
                    if(pid==0){
                        // first child checks command.inFile, others read from input_fd
                        if(i>0){
                            dup2(input_fd, STDIN_FILENO);
                            close(input_fd);
                        }
                        else if (i == 0 && command.inFile != NULL) {//first command
                            int fd_in=open(command.inFile,O_RDONLY);
                            if(fd_in<0){
                                printf("error opening input file\n");
                                exit(1);
                            }
                            dup2(fd_in, STDIN_FILENO);
                            close(fd_in);
                        }
                        
                        // last child must feed the loop
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
                }//simple pipe finsihed
                //in loop part
                for (int k = 0; k < (int)command.loopLen; k++){ // how many times that loop will execute
                    for(int i=0;i<(int)command.inLoop.n;i++){//simple pipe again 
                        int p[2];
                        if (pipe(p) == -1) { printf("fork failed\n");
                            close(p[0]);
                            close(p[1]);
                            exit(1); 
                        }
                        pid_t pid=fork();
                        if(pid<0){
                            printf("fork failed\n");
                            close(p[0]);
                            close(p[1]);
                            exit(1);
                        }
                        //----child
                        if(pid==0){
                            // Input: All children read from input_fd 
                            if(i>0){
                                dup2(input_fd, STDIN_FILENO);
                                close(input_fd);
                            } else {
                                // This is the FIRST command of this pipeline (i == 0)
                                
                                //first command and there is a file to input
                                if (command.before.n == 0 && k == 0 && command.inFile != NULL) {
                                    int fd_in = open(command.inFile, O_RDONLY);
                                    if(fd_in < 0){
                                        printf("error opening input file\n");
                                        exit(1);
                                    }
                                    dup2(fd_in, STDIN_FILENO);
                                    close(fd_in);
                                } 
                                else {
                                    // Not the very first command read from the input_fd
                                    dup2(input_fd, STDIN_FILENO);
                                    close(input_fd);
                                }
                            }
                            
                            // last child for next loop iteraitons
                            if (i < (int)command.inLoop.n - 1) {//inner commands
                                dup2(p[1], STDOUT_FILENO);
                            } 
                            else {
                                // last child for the last loop iteration
                                if (k == (int)command.loopLen - 1 && command.after.n == 0) {
                                    // simple pipline for the last output
                                    if (command.outFile != NULL) {
                                        int fd_out = open(command.outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                        if(fd_out < 0){
                                            printf("error opening output file\n");
                                            exit(1);
                                        }
                                        dup2(fd_out, STDOUT_FILENO);
                                        close(fd_out);
                                    }
                                
                                } 
                                else {
                                    // inner iteration or  there is an after stage
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
            freeCompiledCmd(&command);   
        }//no  parser error and no quit 
    }//close while
    
    free(line);
    freeParser(&parser);
    return 0;

}

