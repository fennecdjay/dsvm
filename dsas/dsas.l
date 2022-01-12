NUM [0-9]
SPACE [ \t\n\r]
%{
#include <ctype.h>
#include "ds.h"
#include "dsas.h"
#include "dsc_parser.h"

#define OP(a)     do { yylval->op = dsop_##a; return DS_OP;  } while(0)
#define BINOP(a)  do { yylval->op = dsop_##a; return DS_BINOP;  } while(0)
#define UNOP(a)   do { yylval->op = dsop_##a; return DS_UNOP;   } while(0)
#define YYSTYPE DSASSTYPE
#define BUMP_SIZE 4096
static char* string_alloc(const char *c);
%}
%option prefix="dsas"
%option bison-bridge
%option reentrant
%option noyywrap noinput nounput

%%

#.*\n        {}

[a-zA-Z_][a-zA-Z_0-9]*":" { yytext[yyleng-1] = '\0'; yylval->id = string_alloc(yytext); return DS_FUN; }

"imm"        { return DS_IMM; }

"+"          { OP(add); }
"-"          { OP(sub); }
"*"          { OP(mul); }
"/"          { OP(div); }
"%"          { OP(mod); }

%{/*
"=="         { BINOP(eq); }
"!="         { BINOP(ne); }
*/%}

"<"          { BINOP(lt); }
"<="         { BINOP(le); }
">"          { BINOP(gt); }
">="         { BINOP(ge); }

"&&"         { BINOP(land); }
"||"         { BINOP(lor);  }

"&"          { BINOP(band); }
"|"          { BINOP(bor);  }
"^"          { BINOP(bxor); }
"<<"         { BINOP(shl); }
">>"         { BINOP(shr);   }
%{/*
"++"         { UNOP(inc); }
"--"         { UNOP(dec); }
"="          { UNOP(mov); }
"!"          { UNOP(not); }
"~"          { UNOP(cmp); }
*/%}
"call"       { return DS_CALL; }
"return"     { return DS_RETURN; }

"if"     { return DS_IF; }
"goto"     { return DS_GOTO; }

"$"{NUM}*      { yylval->num  = strtol(yytext + 1, NULL, 10); return DS_REG; }
":"{NUM}*      { yylval->num  = strtol(yytext + 1, NULL, 10); return DS_LABEL; }
{NUM}*\.{NUM}* { yylval->fnum = strtof(yytext, NULL); return DS_FNUM; }
{NUM}*         { yylval->num  = strtol(yytext, NULL, 10); return DS_NUM; }

[a-zA-Z_][a-zA-Z_0-9]* { yylval->id = string_alloc(yytext); return DS_ID; }

{SPACE}*     {}
.            { puts("unexpected character"); return 1; }
%%
#define BUMP_SIZE 4096
static char _data[BUMP_SIZE];

static char* string_alloc(const char *c) {
  char *data = _data;
  do {
    if(*data == '\0') break;
    if(!strcmp(c, data))
      return data;
  } while((data = strchr(data, '\0') + 1));
  if(data - _data >= BUMP_SIZE)exit(3);
  strcpy(data, c);
  return data;
}

void dsas_init(DsAs *dsas) {
  dsaslex_init(&dsas->scanner);
  dsasset_extra(&dsas, dsas->scanner);
}

bool dsas_compile(DsAs *dsas) {
  dsas->stmts = stmt_start();
  return dsasparse(dsas);
}

bool dsas_file(DsAs *dsas, FILE *file) {
  dsasset_in(file, dsas->scanner);
  const bool ret = dsas_compile(dsas);
  fclose(file);
  return ret;
}

bool dsas_filename(DsAs *dsas, char* path) {
  FILE *file= path ? fopen(path, "r") : stdin;
  return file ? dsas_file(dsas, file) :false;
}

void dsas_destroy(DsAs *const dsas) {
  dsaslex_destroy(dsas->scanner);
}
