#ifndef GLOBAL_PARSER_H
#define GLOBAL_PARSER_H
typedef enum { C_PLAIN, C_VOID, C_AND, C_OR, C_PIPE, C_SEQ } cmdtype;

struct cmd {
	int type;
	struct cmd *left;
	struct cmd *right;

	char **args;
	char *input;
	char *output;
	char *append;
	char *error;
};

struct arglist {
	char *arg;
	struct arglist *next;
};

extern struct cmd* parser (char*);
#endif

