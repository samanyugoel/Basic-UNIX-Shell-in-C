# A Custom UNIX Shell in C

## Overview

A custom-built shell created in C as a learning project which demonstrates dynamic memory allocation, memory hygiene, error and signal handling, tokenization and string parsing.

### Features

* **Dynamic Memory Allocation** - Allocates memory for commands and argument lists of varying lengths using `malloc` and dynamic `realloc` arrays.

* **Memory Hygiene** - Safely handles the allocated memory and freeing it wherever required using a `cleanup()` function for easy memory cleaup.
 
* **Command Execution** - Safely forks child processes and executes system executables (e.g., `ping`, `grep`, `mkdir`, etc).

* **Built-in Commands** - 
  * `cd`: Changes directories natively in the parent process, falling back to the `HOME` environment variable if no path is provided.
  * `ls`: Custom directory reader that lists files cleanly and automatically filters out hidden files.
  * `exit`: Safely frees all dynamically allocated memory before terminating.

* **Error and Signal Handling** - Properly exits or gives proper signal when some errror occurs.

* **Tokenization and String Parsing** - User arguments are tokenized parsed using `getword()` function.


### The Files

* `Basic_UNIX_Shell.c` - The main C file containing the source code. 

* `Makefile` - Easy building of the code for easy compiling, cleaning and memory testing.

* `.gitignore` - Ignores the executable from tracking.

* `README.md` - Project Documentation.


## How to Compile and Run

Make sure you have `gcc`, `make` and `valgrind` installed on your Linux environment.

### Compile the Shell

* Type `make` to generate the `shell` executable.

* Run the shell with `./shell`.

* Test memory leaks and errors with `make valgrind`.

* Use `make clean` to remove the compiled executable.


## Lesson Learned

The developing of this shell has provided a deep insight on how the UNIX shell actually functions, majorly the functionality of `fork()` and `exec()` commands. This also allowed me to understand the drawbacks and advantages of dynamic memory allocation as well as securely applying it. 
