#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <error.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>

#define INPUT_BUFFER 100
//the initial input buffer

void cleanup(char *input, char **args, int count);
char *getword(int x, char *input);     
int pipe_function(int pipe_implement, char *input, char **args, int z);
//z is the total number of arguments

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    while(1)    
    //infinite loop so the shell works after finishing the execution of any command 
    {
        printf("shell> ");
        fflush(stdout);
        
        int i, z, x;
        i = x = z = 0;
        int args_buffer = 20;
        char *input = malloc(INPUT_BUFFER * sizeof(char));   
        char **args = malloc(args_buffer * sizeof(char*));
        //dynamic memory allocation used to allocate memory for the input of arguments


        if (input == NULL || args == NULL){
            //error handling for allocation errors
            fprintf(stderr, "Memory Allocation Error.\n");
            return 1;
        }


        if (fgets(input, INPUT_BUFFER * sizeof(char), stdin) == NULL){  
            //frees the memory if error with fetching the user input
            cleanup(input, args, z);
            return 1;
        }
        
        while (input[x] != '\0'){
            if (isspace((unsigned char)input[x])){
                //skips the spaces for proper tokenization
                x++;
            }
            else{
                if (z >= args_buffer - 1) {
                    args_buffer *= 2; 

                    char **temp = realloc(args, args_buffer * sizeof(char*));
                    //reallocate memory if no more space is left for parsing the input
                    if (temp == NULL) {
                        fprintf(stderr, "Memory Allocation Error during realloc.\n");
                        cleanup(input, args, z);
                        return 1;
                    }
                    args = temp;
                }
                args[z] = getword(x, &input[0]);
                //calls the getword() for tokenized output stored in an array
                x += strlen(args[z]);
                z++;
            }
        }

        args[z] = NULL;
        //sets the last place in the array to NULL so as to run the exec() properly as it requires a NULL argument in the end

        if (args[0] == NULL) {
            //frees memory if no character is entered
            cleanup(input, args, z);
            continue; 
        }

        int pipe_implement = -1;
        while (args[i] != NULL){
            if (strcmp(args[i], "|") == 0){
                pipe_implement = i;
                //sets the variable to the position of the pipe, which allows easy control over changes and access to that slot of the array
                break;
                //break exits the while loop whenever a pipe is found
            }
            i++;
        }

        if(pipe_implement != -1)
        //this won't execute unless pipe is called for
        {
            int pipe_error = pipe_function(pipe_implement, input, args, z);
            if (pipe_error != 0){
                fprintf(stderr, "Pipe failed.\n");
                cleanup(input, args, z);
                exit(2);
            }
        }

        else{
            if (args[0] != NULL && 
            strcmp(args[0], "ls") != 0 && 
            strcmp(args[0], "cd") != 0 && 
            strcmp(args[0], "exit") != 0)
            {
                int pid = fork();
                //fork the process for running the exec() command

                if (pid == -1){
                    cleanup(input, args, z);
                    return 2;
                }

                if (pid == 0){      
                    //child process
                    //gets destroyed after exec() runs without error
                    int err = execvp(args[0], args);

                    if (err == -1){
                    //gives error if does not properly
                        error(1, errno, "Command execution failed.\n");
                        exit(1);
                    }
                }
                else {
                    //parent process
                    //if parent process was destroyed instead of the child process, the shell would have destroyed itself as well, and would result in only letting one command run
                    int statusofexec;
                    wait(&statusofexec);
                    if(WIFEXITED(statusofexec)){
                        int statuscode = WEXITSTATUS(statusofexec);
                        //gives status code depending on whether the child process exited or not
                        if (statusofexec != 0){
                            fprintf(stderr, "Command failed with status code %d.\n", statuscode);      
                        }
                    }
                }
            }

            else if (strcmp(args[0], "ls") == 0){
                char *targetdir = (args[1] != NULL) ? args[1] : ".";
                    
                DIR *dir = opendir(targetdir);
                if (dir == NULL) {
                    // If the directory doesn't exist or we don't have permission
                    fprintf(stderr, "ls: Cannot open '%s': %s\n", targetdir, strerror(errno));
                } 
                else {
                    struct dirent *entry;
                    //using built-in struct for accessing the directory
                    // Read every file entry one by one
                    while ((entry = readdir(dir)) != NULL) {
                        if (entry->d_name[0] == '.') {
                            //skips the hidden files
                            continue;
                    }

                    printf("%s  ", entry->d_name);

                    }
                    
                    printf("\n");
                    closedir(dir);
                }
            }

            else if (strcmp(args[0], "cd") == 0){
                char *targetdir = args[1];

                if (targetdir == NULL){
                    targetdir = getenv("HOME");
                }
                        
                if (chdir(targetdir) != 0){
                    fprintf(stderr, "Command execution failed.\n");
                }
            }


            else if (strcmp(args[0], "exit") == 0){
                printf("Exit successfully initiated.");
                //exits succesfully and frees used memory
                cleanup(input, args, z);
                return 0;
            }
        }
        cleanup(input, args, z);
        //cleans up each time the infinite loop runs
    }
    return 0;
}

int pipe_function(int pipe_implement, char *input, char **args, int z){

    if (args[0] == NULL){
        fprintf(stderr, "Syntax error : expected command before '|' \n");
        return 0;       
    }

    if (args[pipe_implement + 1] == NULL){
        fprintf(stderr, "Syntax error : expected command after '|' \n");
        return 0;
    }
    free(args[pipe_implement]);
    //frees the '|' from the array
    args[pipe_implement] = NULL;
    //sets that slot to NULL for exec()
    int pid1 = -1;
    int pid2 = -1;
    int fd[2];  
    //0 -> input 
    //1 -> output

    if (pipe(fd) == -1){
        return -1;
    }

        //first process in pipe
    pid1 = fork();
    if (pid1 == -1){
        fprintf(stderr, "pipe() failed\n");
        close(fd[0]);
        close(fd[1]);
        return 3;
    }
    if (pid1 == 0)
    {
        //child process
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        close(fd[0]);

            if (args[0] != NULL && 
            strcmp(args[0], "ls") != 0 && 
            strcmp(args[0], "cd") != 0 && 
            strcmp(args[0], "exit") != 0)
            {
                int err1 = execvp(args[0], args);

                if (err1 == -1){
                    error(1, errno, "Command execution failed.\n");
                    exit(1);
                }
            }
        else if (strcmp(args[0], "ls") == 0){
            char *targetdir = (args[1] != NULL) ? args[1] : ".";
                    
            DIR *dir = opendir(targetdir);
            if (dir == NULL) {
                // If the directory doesn't exist or we don't have permission
                fprintf(stderr, "ls: Cannot open '%s': %s\n", targetdir, strerror(errno));
                exit(1);
            } 
            else {
                struct dirent *entry;            
                // Read every file entry one by one
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_name[0] == '.') {
                            continue;
                    }
                    if (isatty(STDOUT_FILENO)) {
                        printf("%s  ", entry->d_name);
                    } 
                    
                    else{
                        printf("%s\n", entry->d_name);
                    }
                }
                printf("\n"); 
                closedir(dir);
                exit(0);
            }
        }

        else if (strcmp(args[0], "cd") == 0){
            char *targetdir = args[1];

            if (targetdir == NULL){
                targetdir = getenv("HOME");
            }   
                            
            if (chdir(targetdir) != 0){
                fprintf(stderr, "Command execution failed.\n");
            }
            exit(0);
        }


        else if (strcmp(args[0], "exit") == 0){
            printf("Exit successfully initiated.");
            cleanup(input, args, z);
            exit(0);
        }
    }

    //second process in pipe
    pid2 = fork();
    if (pid2 == -1){
        fprintf(stderr, "pipe() failed\n");
        close(fd[0]);
        close(fd[1]);
        return 3;
    }
                    
    if (pid2 == 0)
    {
        //child process
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if (args[pipe_implement + 1] != NULL && 
        strcmp(args[pipe_implement + 1], "ls") != 0 && 
        strcmp(args[pipe_implement + 1], "cd") != 0 && 
        strcmp(args[pipe_implement + 1], "exit") != 0){
            int err2 = execvp(args[pipe_implement + 1], &args[pipe_implement + 1]);

            if (err2 == -1){
                error(1, errno, "Command execution failed.\n");
                cleanup(input, args, z);
                exit(1);
            }
        }
        else if (strcmp(args[pipe_implement + 1], "ls") == 0){
            char *targetdir = (args[pipe_implement + 2] != NULL) ? args[pipe_implement + 2] : ".";
                        
            DIR *dir = opendir(targetdir);
            if (dir == NULL) {
                // If the directory doesn't exist or we don't have permission
                fprintf(stderr, "ls: Cannot open '%s': %s\n", targetdir, strerror(errno));
                exit(1);
            } 
            else {
                struct dirent *entry;
                                    
                // Read every file entry one by one
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_name[0] == '.') {
                        continue;
                    }
                    printf("%s  ", entry->d_name);
                }
                printf("\n"); 
                closedir(dir);
                exit(0);
            }
        }

        else if (strcmp(args[pipe_implement + 1], "cd") == 0){
            char *targetdir = args[pipe_implement + 2];

            if (targetdir == NULL){
                targetdir = getenv("HOME");
            }
                                    
            if (chdir(targetdir) != 0){
                fprintf(stderr, "Command execution failed.\n");
            }
            exit(0);
        }


        else if (strcmp(args[pipe_implement + 1], "exit") == 0){
            //new command starts after the NULL which is at pipe_implement
            printf("Exit successfully initiated.");
            close(fd[0]);
            close(fd[1]);         
            exit(0);
        }
    }
        
    close(fd[0]);
    close(fd[1]);

    int status1 = 0;
    int status2 = 0;
    //for storing the status from both processes

    if (pid1 > 0) {
        waitpid(pid1, &status1, 0);
    } 
    if (pid2 > 0) {
        waitpid(pid2, &status2, 0);
    }

    if ((WIFEXITED(status2) && WEXITSTATUS(status2)) || (WIFEXITED(status1) && WEXITSTATUS(status1)) != 0) {
        fprintf(stderr, "Command failed.\n");
    }
    return 0;
}

void cleanup(char *input, char **args, int count){
if (args != NULL) {
        
        for (int k = 0; k < count; k++) {
            //changed counter since it won't free NULL if it used instead
            free(args[k]);
            //frees each slot of the array

        }
        free(args);
        //frees the array as a whole in structure
    }
    if (input != NULL) {
        free(input);
        //frees the input used to store the arguments
    }
}
char *getword(int x, char *input){
    int start = x;
    int len = 0;
    
    while (input[x] != '\n' && input[x] != '\0' && !isspace((unsigned char)input[x])){
        x++;
        len++;
    }
    char *arr = malloc(sizeof(char) * (len + 1));
    //changed allocation logic since it caused a buffer overflow before and leaves one space for '\0' so the string always ends properly
    if (arr == NULL){
        fprintf(stderr, "Memory Allocation Error\n");
        exit(1);
    }
    memcpy(arr, &input[start], len);
    arr[len] = '\0';
    //sets last character of the array to '\0' so as to end the array containing the string
    return arr;
}
