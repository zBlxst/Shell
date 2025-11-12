#ifndef BUILTINS_H
#define BUILTINS_H

#include "global_parser.h"

int builtin_cd(struct cmd* cmd);
int builtin_exit(struct cmd* cmd);

#endif