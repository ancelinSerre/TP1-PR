/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "readcmd.h"

int main()
{
	struct cmdline *command;
	int i, j, pid, child, dead, status;

	/* Looping while user doesn't want to exit */
	while (1) {

		printf("shell> ");
		command = readcmd();

		pid = fork();
		if (pid < 0) {
			perror("shell error: fork failed !\n");
			exit(-1);
		} else if (pid == 0) {
			/* Child code */
			
			/* Handling */
			if (command->out) {
				/* Open the output file */
				int out = open(command->out, O_RDWR|O_CREAT|O_APPEND);
				if (-1 == out) { 
					perror("opening the output file failed"); 
					exit(-1); 
				}

				/* Redirect stdout to a file */
				if (-1 == dup2(out, STDOUT_FILENO)) { 
					perror("cannot redirect to stdout"); 
					exit(-1); 
				}		
				close(out);
			}
			
			if (command->in) {
				/* Open the input file */
				int in = open(command->in, O_RDONLY);
				if (-1 == in) {
					perror("opening the input file failed\n");
					exit(-1);
				}

				/* Redirect in to stdin */
				if (-1 == dup2(in, STDIN_FILENO)) {
					perror("cannot redirect to stdin");
					exit(-1);
				}
				close(in);

			}
			
			if(execvp(command->seq[0][0], command->seq[0]) == -1) {
				perror("shell error on execvp \n");
				exit(-1);
			}

		} else {
			/* Father code */
			child = 1;
			while(child) {
				dead = wait(&status);
				if(dead == pid) child = 0;
			}

		}

		/* If input stream closed, normal termination */
		if (!command) {
			printf("exit\n");
			exit(0);
		}

		if (command->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", command->err);
			continue;
		}

		if (command->in) printf("in: %s\n", command->in);
		

		/* Display each command of the pipe */
		for (i=0; command->seq[i]!=0; i++) {
			char **cmd = command->seq[i];
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}
			printf("\n");
		}
	}
}
