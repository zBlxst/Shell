%{

#include <string.h>

%}

%option nounput
%option noinput
%option noyywrap

%%

"("				{ return LPAR; }
")"             { return RPAR; }
";"             { return SEMI; }
"&&"            { return AND; }
"||"            { return OR; }
"|"             { return PIPE; }
">"             { return OUTPUT; }
"<"             { return INPUT; }
"2>"            { return ERROR; }
">>"            { return APPEND; }

[0-9A-z*?/.-]*  {
		  yylval.string = strdup(yytext);
		  return ARG;
		}
\"[^"]*\"       {
		  yylval.string = strdup(yytext+1);
		  yylval.string[strlen(yylval.string)-1] = 0;
		  return ARG;
		}
\'[^']*\'       {
		  yylval.string = strdup(yytext+1);
		  yylval.string[strlen(yylval.string)-1] = 0;
		  return ARG;
		}

.               ;
