#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <mcheck.h>

#include "parser.h"
#include "shell.h"

/**
 * Program that simulates a simple shell.
 * The shell covers basic commands, including builtin commands 
 * (cd and exit only), standard I/O redirection and piping (|). 
 
 */

#define MAX_DIRNAME 100
#define MAX_COMMAND 1024
#define MAX_TOKEN 128

/* Functions to implement, see below after main */
int execute_cd(char** words);
int execute_nonbuiltin(simple_command *s);
int execute_simple_command(simple_command *cmd);
int execute_complex_command(command *cmd);


int main(int argc, char** argv) {
	
	char cwd[MAX_DIRNAME];           /* Current working directory */
	char command_line[MAX_COMMAND];  /* The command */
	char *tokens[MAX_TOKEN];         /* Command tokens (program name, 
					  * parameters, pipe, etc.) */

	while (1) {

		/* Display prompt */		
		getcwd(cwd, MAX_DIRNAME-1);
		printf("%s> ", cwd);
		
		/* Read the command line */
		fgets(command_line, MAX_COMMAND, stdin);
		/* Strip the new line character */
		if (command_line[strlen(command_line) - 1] == '\n') {
			command_line[strlen(command_line) - 1] = '\0';
		}
		
		/* Parse the command into tokens */
		parse_line(command_line, tokens);

		/* Check for empty command */
		if (!(*tokens)) {
			continue;
		}
		
		/* Construct chain of commands, if multiple commands */
		command *cmd = construct_command(tokens);
		//print_command(cmd, 0);
    
		int exitcode = 0;
		if (cmd->scmd) {
			exitcode = execute_simple_command(cmd->scmd);
			if (exitcode == -1) {
				break;
			}
		}
		else {
			exitcode = execute_complex_command(cmd);
			if (exitcode == -1) {
				break;
			}
		}
		release_command(cmd);
	}
    
	return 0;
}


/**
 * Changes directory to a path specified in the words argument;
 * For example: words[0] = "cd"
 *              words[1] = "csc209/assignment3/"
 * Your command should handle both relative paths to the current 
 * working directory, and absolute paths relative to root,
 * e.g., relative path:  cd csc209/assignment3/
 *       absolute path:  cd /u/bogdan/csc209/assignment3/
 */
int execute_cd(char** words) {
	
	/** 
	 * TODO: 
	 * The first word contains the "cd" string, the second one contains 
	 * the path.
	 * Check possible errors:
	 * - The words pointer could be NULL, the first string or the second 
	 *   string could be NULL, or the first string is not a cd command
	 * - If so, return an EXIT_FAILURE status to indicate something is 
	 *   wrong.
	 */
	 if (words == NULL || strcmp(words[0], "cd") != 0) {
	     return EXIT_FAILURE;
	 }

	/**
	 * TODO: 
	 * The safest way would be to first determine if the path is relative 
	 * or absolute (see is_relative function provided).
	 * - If it's not relative, then simply change the directory to the path 
	 * specified in the second word in the array.
	 * - If it's relative, then make sure to get the current working 
	 * directory, append the path in the second word to the current working
	 * directory and change the directory to this path.
	 * Hints: see chdir and getcwd man pages.
	 * Return the success/error code obtained when changing the directory.
	 */
	 
	 /* Add relative path to buf if the path is not absolute. */
	 char buf[MAX_DIRNAME];
	 int status;
	 getcwd(buf, MAX_DIRNAME - 1);
	 
	 if (is_relative(words[1])) {
	     strcat(buf, "/");
	     strcat(buf, words[1]);
	     status = chdir(buf);
	 } else {
         status = chdir(words[1]);
	 }
	 if (status != 0) {
	     perror(words[1]);
	 }

    return 0;	 
}


/**
 * Executes a program, based on the tokens provided as 
 * an argument.
 * For example, "ls -l" is represented in the tokens array by 
 * 2 strings "ls" and "-l", followed by a NULL token.
 * The command "ls -l | wc -l" will contain 5 tokens, 
 * followed by a NULL token. 
 */
int execute_command(char **tokens) {
	
	/**
	 * TODO: execute a program, based on the tokens provided.
	 * The first token is the command name, the rest are the arguments 
	 * for the command. 
	 * Hint: see execlp/execvp man pages.
	 * 
	 * - In case of error, make sure to use "perror" to indicate the name
	 *   of the command that failed.
	 *   You do NOT have to print an identical error message to what would 
	 *   happen in bash.
	 *   If you use perror, an output like: 
	 *      my_silly_command: No such file of directory 
	 *   would suffice.
	 * Function returns only in case of a failure (EXIT_FAILURE).
	 */
	 
	 /* Simply execute commands, if failed, return EXIT_FAILURE */
     execvp(*tokens, tokens);
     perror(tokens[0]);
     return EXIT_FAILURE;

}


/**
 * Executes a non-builtin command.
 */
int execute_nonbuiltin(simple_command *s) {
	/**
	 * TODO: Check if the in, out, and err fields are set (not NULL),
	 * and, IN EACH CASE:
	 * - Open a new file descriptor (make sure you have the correct flags,
	 *   and permissions);
	 * - redirect stdin/stdout/stderr to the corresponding file.
	 *   (hint: see dup2 man pages).
	 * - close the newly opened file descriptor in the parent as well. 
	 *   (Avoid leaving the file descriptor open across an exec!) 
	 * - finally, execute the command using the tokens (see execute_command
	 *   function above).
	 * This function returns only if the execution of the program fails.
	 */
	
	/* Create file descriptor for stdin/stdout/stderr */
	if (s->in != NULL) {
	    int fd1 = open(s->in, O_RDONLY);
	    if(fd1 == -1) { perror("Could not open file."); }
	    dup2(fd1, STDIN_FILENO);
	    close(fd1);
	}
	
	if (s->out != NULL) {
	    int fd2 = open(s->out, O_CREAT | O_WRONLY, 0644);
	    if(fd2 == -1) { perror("Could not open file."); }
	    dup2(fd2, STDOUT_FILENO);
	    close(fd2);
	}
	
	if (s->err != NULL) {
	    int fd3 = open(s->err, O_CREAT | O_WRONLY, 0644);
	    if(fd3 == -1) { perror("Could not open file."); }
	    dup2(fd3, STDERR_FILENO);
	    close(fd3);
	}

    /* Pass tokens to execute_commands */
    int status = execute_command(s->tokens);
    if (status != 0) {
        return status;
    }
    
    return 0;
}


/**
 * Executes a simple command (no pipes).
 */
int execute_simple_command(simple_command *cmd) {

	/**
	 * TODO: 
	 * Check if the command is builtin.
	 * 1. If it is, then handle BUILTIN_CD (see execute_cd function provided) 
	 *    and BUILTIN_EXIT (simply exit with an appropriate exit status).
	 * 2. If it isn't, then you must execute the non-builtin command. 
	 * - Fork a process to execute the nonbuiltin command 
	 *   (see execute_nonbuiltin function above).
	 * - The parent should wait for the child.
	 *   (see wait man pages).
	 */
	 
	 /* Execute builtin commands */
	 switch(cmd->builtin) {
	     case 1:
	         /* Execute builtin command */
	         return execute_cd(cmd->tokens);
	     case 2:
             /* Simply exit from the shell */
	         return -1;
	}
	
	/* fork() process and let child run execute_nonbuiltin */
	//int stat;
    switch (fork()) {
        case -1:
            perror("fork");
            return EXIT_FAILURE;
        
        case 0:
            execute_nonbuiltin(cmd);
            exit(EXIT_SUCCESS);
        
        default:
            wait(NULL);   /* wait for child and return */

    }
	return 0;
}


/**
 * Executes a complex command.  A complex command is two commands chained 
 * together with a pipe operator.
 */
int execute_complex_command(command *c) {
	
	/**
	 * TODO:
	 * Check if this is a simple command, using the scmd field.
	 * Remember that this will be called recursively, so when you encounter
	 * a simple command you should act accordingly.
	 * Execute nonbuiltin commands only. If it's exit or cd, you should not 
	 * execute these in a piped context, so simply ignore builtin commands. 
	 */
    
    if (c->scmd != NULL) {	
		/* Pass simple commands to nonbuiltin commands */
	    execute_nonbuiltin(c->scmd);
	}
	/** 
	 * Optional: if you wish to handle more than just the 
	 * pipe operator '|' (the '&&', ';' etc. operators), then 
	 * you can add more options here. 
	 */

	if (!strcmp(c->oper, "|")) {
		
		/**
		 * TODO: Create a pipe "pfd" that generates a pair of file 
		 * descriptors, to be used for communication between the 
		 * parent and the child. Make sure to check any errors in 
		 * creating the pipe.
		 */

			
		/**
		 * TODO: Fork a new process.
		 * In the child:
		 *  - close one end of the pipe pfd and close the stdout 
		 * file descriptor.
		 *  - connect the stdout to the other end of the pipe (the 
		 * one you didn't close).
		 *  - execute complex command cmd1 recursively. 
		 * In the parent: 
		 *  - fork a new process to execute cmd2 recursively.
		 *  - In child 2:
		 *     - close one end of the pipe pfd (the other one than 
		 *       the first child), and close the standard input file 
		 *       descriptor.
		 *     - connect the stdin to the other end of the pipe (the 
		 *       one you didn't close).
		 *     - execute complex command cmd2 recursively. 
		 *  - In the parent:
		 *     - close both ends of the pipe. 
		 *     - wait for both children to finish.
		 */
		int pfd[2];

        if(pipe(pfd) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }
            
        switch (fork()) {
            case -1:
                perror("fork 1");
                return EXIT_FAILURE;

            /* Child 1 */
            case 0:
                if (close(pfd[0]) == -1) {    /* Close stdin */
                    perror("close pfd_1 child");
                    return EXIT_FAILURE;
                }
                if (pfd[1] !=  STDOUT_FILENO) {
                    /* duplicate child process */
                    if (dup2(pfd[1],  STDOUT_FILENO) == -1) {
                        perror("dup2 child 1");
                        return EXIT_FAILURE;
                    }
                    if (close(pfd[1]) == -1) {  /* Close the pipe */
                    perror("close pfd child 1");
                    return EXIT_FAILURE;
                    }
                }
                execute_complex_command(c->cmd1);
                exit(EXIT_SUCCESS);
            
            /* Parent */
            default:
                switch(fork()) {
                    case -1:
                        perror("fork 2");
                        return EXIT_FAILURE;
                    
                    /* Child 2 */
                    case 0:
                        if (close(pfd[1]) == -1) {    /* Close stdout */
                            perror("close pfd child 2");
                            return EXIT_FAILURE;
                        }
                        if (pfd[0] !=  STDIN_FILENO) {
                            /* duplicate child process */
                            if (dup2(pfd[0],  STDIN_FILENO) == -1) {
                                perror("dup2 child 2");
                                return EXIT_FAILURE;
                            }
                            if (close(pfd[0]) == -1) {  /* Close the pipe */
                            perror("close pfd child 2");
                            return EXIT_FAILURE;
                            }
                        }
                        /* Recursively execute command 2 */
                        execute_complex_command(c->cmd2);
                        exit(EXIT_SUCCESS);
                    
                    default:
                        /* Close parent pipes */
						if (close(pfd[0]) == -1) {
							 perror("close pfd_0 parent");
							 return EXIT_FAILURE;
						 }
						 if (close(pfd[1]) == -1) {
					          perror("close pfd_1 parent");
					          return EXIT_FAILURE;
				         }
                         wait(NULL);  /* Wait for child 1 to exit */
                         wait(NULL);  /* Wait for child 2 to exit */
                
                }

            }

	}
	return 0;
}
