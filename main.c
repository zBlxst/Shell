#include "main.h"
#include "constants.h"
#include "builtins.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <readline/readline.h>


const char red[] = "\e[1;31m";
const char green[] = "\e[1;32m";
const char blue[] = "\e[1;34m";
const char reset_color[] = "\e[0m";

void apply_redirects(struct cmd *cmd) {
	int fd;
	if (cmd->input || cmd->output || cmd->append || cmd->error)
	{
		if (cmd->input) {
			if ((fd = open(cmd->input, O_RDONLY)) == -1) {
				printf("Cannot open %s\n", cmd->input);
				exit(-1);
			}
			dup2(fd, 0);

		}
		if (cmd->output) {
			if ((fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->output);
				exit(-1);
			}
			dup2(fd, 1);
		}
		if (cmd->append) {
			if ((fd = open(cmd->append, O_WRONLY | O_CREAT | O_APPEND, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->append);
				exit(-1);
			}
			dup2(fd, 1);
		}
		if (cmd->error) {
			if ((fd = open(cmd->error, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
				printf("Cannot open %s\n", cmd->error);
				exit(-1);
			}
			dup2(fd, 2);
		}
	}
}

int execute_builtin(struct cmd *cmd) {
	if (!strcmp(cmd->args[0], "cd")) return builtin_cd(cmd);
	if (!strcmp(cmd->args[0], "exit")) return builtin_exit(cmd);
    if (!strcmp(cmd->args[0], "bubulles")) return builtin_bubulles(cmd);
	return NOT_BUILTIN;
}

char* get_prompt() {
	char* prompt = calloc(1, BUFFER_SIZE);
	char temp[BUFFER_SIZE];
	char temp2[BUFFER_SIZE];

	char* ps1 = getenv("PS1");
	strncpy(prompt, ps1, BUFFER_SIZE);

	prompt[BUFFER_SIZE-1] = '\0';
	
	char* old_ptr;
	char* ptr;

	// \\ replaced by \xca\xfe
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\\\\"))) { 
		ptr[0] = '\xca';
		ptr[1] = '\xfe';
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);

	// \u
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\\u"))) { 
		strncat(prompt, old_ptr, ptr-old_ptr);

		strncat(prompt, getpwuid(geteuid())->pw_name, BUFFER_SIZE);

		ptr[0] = '\0';
		ptr[1] = '\0';
		old_ptr = ptr+2;
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);
	
	// \h
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\\h"))) { 
		strncat(prompt, old_ptr, ptr-old_ptr);
		
		gethostname(temp2, BUFFER_SIZE);
		strncat(prompt, temp2, BUFFER_SIZE);

		ptr[0] = '\0';
		ptr[1] = '\0';
		old_ptr = ptr+2;
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);

	// \$
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\\$"))) { 
		strncat(prompt, old_ptr, ptr-old_ptr);

		if (geteuid() == 0) strncat(prompt, "#", BUFFER_SIZE);
		else strncat(prompt, "$", BUFFER_SIZE);
		
		ptr[0] = '\0';
		ptr[1] = '\0';
		old_ptr = ptr+2;
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);
	
	// \w
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\\w"))) { 
		strncat(prompt, old_ptr, ptr-old_ptr);

		getcwd(temp2, BUFFER_SIZE);
		str_replace_home(temp2);
		strncat(prompt, temp2, BUFFER_SIZE);		

		ptr[0] = '\0';
		ptr[1] = '\0';
		old_ptr = ptr+2;
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);

	// \xca\xfe replaced by \\ -
	strcpy(temp, prompt);
	memset(prompt, 0, BUFFER_SIZE);
	old_ptr = temp;
	ptr = temp+BUFFER_SIZE;
	while ((ptr = strstr(temp, "\xca\xfe"))) { 
		ptr[0] = '\\';
		ptr[1] = '\\';
	}
	strncat(prompt, old_ptr, BUFFER_SIZE);


	return prompt;

}

int execute(struct cmd *cmd) {
	switch (cmd->type)
	{
		int status;
		int pid;
		int pid2;
		int pipefd[2];
	    case C_PLAIN:
			if ((status = execute_builtin(cmd)) != NOT_BUILTIN) {
				return status;
			}
			pid = fork();
			if (pid == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid == 0) {
				apply_redirects(cmd);
				if (execvp(cmd->args[0], cmd->args) == -1) {
					fprintf(stderr, "Can't execute command %s\n", cmd->args[0]);
				}
				exit(-1);
			}
			wait(&status);
			return status;

		case C_SEQ:
			execute(cmd->left);
			return execute(cmd->right);
		case C_AND:
	    	status = execute(cmd->left);
			return status ? status : execute(cmd->right);
		case C_OR:
	    	status = execute(cmd->left);
			return status ? execute(cmd->right) : status;
		case C_PIPE:
			if (pipe(pipefd) == -1) {
				perror("pipefd");
				exit(-1);
			}

			pid = fork();
			if (pid == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid == 0) {
				close(pipefd[0]);
				dup2(pipefd[1], 1);
				status = execute(cmd->left);
				close(pipefd[1]);
				exit(status);
			}

			pid2 = fork();
			if (pid2 == -1) {
				perror("fork");
				exit(-1);
			}
			if (pid2 == 0) {
				close(pipefd[1]);
				dup2(pipefd[0], 0);
				status = execute(cmd->right);
				close(pipefd[0]);
				exit(status);
			}

			waitpid(pid, NULL, 0);
			close(pipefd[0]);
			close(pipefd[1]);
			waitpid(pid2, &status, 0);
			return status;
			
		case C_VOID:
			int stdinfd = dup(0);
			int stdoutfd = dup(1);
			int stderrfd = dup(2);
			apply_redirects(cmd);
			status = execute(cmd->left);
			dup2(stdinfd, 0);
			dup2(stdoutfd, 1);
			dup2(stderrfd, 2);
			return status;

		default:
			return -1;
	}
}

void str_replace_home(char *path) {
	char *home = getpwuid(getuid())->pw_dir;
	char *tmp = malloc(BUFFER_SIZE);
	size_t home_length = strlen(home);
	home_length = (home_length < BUFFER_SIZE) ? home_length : BUFFER_SIZE;
	if (!strncmp(home, path, home_length)) {
		snprintf(tmp, BUFFER_SIZE, "~%s", path+home_length);
		strncpy(path, tmp, BUFFER_SIZE);
	}
	free(tmp);
}

int exit_shell(int status) {
	exit(status);
}

int main(int argc, char **argv) {
	setenv("PS1", "In shell : \\u@\\h:\\w\\$ ", 0);

	printf("Welcome to zblxst's shell!\n");

	while (1)
	{
		char* prompt = get_prompt();
		char* line = readline(prompt);
		free(prompt);
		if (!line) exit_shell(0);
		if (!*line) continue;
				

		struct cmd *cmd = parser(line);
		if (!cmd) continue;
		execute(cmd);
	}
}
