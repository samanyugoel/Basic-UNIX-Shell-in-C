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


void cleanup(char *input, char **args);
char *getword(int x, char *input);      

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    while(1)
    //infinite loop so the shell works after finishing the execution of any command                    
    {
        printf("shell> ");
        fflush(stdout);         
        
        int z, x;
        x = z = 0;
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
            cleanup(input, args);
            return 1;
        }
        
        while (input[x] != '\0') {
            if (isspace(input[x])){
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
                        cleanup(input, args);
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
            cleanup(input, args);
            continue; 
        }

        
        if (args[0] != NULL && 
        strcmp(args[0], "ls") != 0 && 
        strcmp(args[0], "cd") != 0 && 
        strcmp(args[0], "exit") != 0)
        {
            int pid = fork();
            //fork the process for running the exec() command

            if (pid == -1){
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
            printf("Exit successfully initiated.\n");
            //exits succesfully and frees used memory
            cleanup(input, args);
            return 0;
        }
    cleanup(input, args);
    }
    return 0;
}

void cleanup(char *input, char **args){
if (args != NULL) {
        
        for (int k = 0; args[k] != NULL; k++) {
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
    int y;
    char *arr = malloc(sizeof(char) * 100);
    for (y = 0 ; input[x] != '\n' && input[x] != '\0' && !isspace(input[x]); y++){  
        arr[y] = input[x];
        x++;
    }
    arr[y] = '\0';
    //sets last character of the array to '\0' so as to end the array of string
    return arr;
}
