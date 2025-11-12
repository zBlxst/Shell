#include "builtins.h"

#include "main.h"

#include <sys/types.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int builtin_cd(struct cmd* cmd) {
    if (cmd->args[1] == NULL) {
        if (chdir(getpwuid(getuid())->pw_dir) == -1) {
            printf("Cannot access %s\n", getpwuid(getuid())->pw_dir);
            return -2;
        }
    } else {
        if (chdir(cmd->args[1]) == -1) {
            printf("Cannot access %s\n", cmd->args[1]);
            return -2;
        }
    }
    return 0;
}

int builtin_exit(struct cmd* cmd) {
    if (!cmd->args[1]) {
        exit_shell(0);
    }
    exit_shell(atoi(cmd->args[1]));
    return 0;
}