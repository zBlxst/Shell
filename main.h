#ifndef MAIN_H
#define MAIN_H

#include "global_parser.h"

int execute(struct cmd *cmd);
void apply_redirects(struct cmd *cmd);
int execute_builtin(struct cmd *cmd);
int exit_shell(int status);
void str_replace_home(char *path);

#endif 