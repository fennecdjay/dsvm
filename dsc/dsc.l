NUM [0-9]
SPACE [ \t\n\r]
%{
#include <ctype.h>
#include "ds.h"
#include "dsc.h"
#include "dsc_parser.h"

#define OP(a)     do { yylval->op = dsop_##a; return DS_OP;  } while(0)
#define BINOP(a)  do { yylval->op = dsop_##a; return DS_BINOP;  } while(0)
#define IBINOP(a) do { yylval->op = dsop_##a; return DS_IBINOP; } while(0)
#define UNOP(a)   do { yylval->op = dsop_##a; return DS_UNOP;   } while(0)
#define _JUMP()   do { yylval->op = dsop_jump; return DS_JUMP;   } while(0)
#define JUMP(a)   do { yylval->op = dsop_##a##_jump; return DS_JUMP;   } while(0)
#define YYSTYPE DSSTYPE
#define BUMP_SIZE 4096
static char* string_alloc(const char *c);
%}
%option prefix="ds"
%option bison-bridge
%option reentrant
%option noyywrap noinput nounput

%%

#.*\n        {}

[a-zA-Z_][a-zA-Z_0-9]*":" { yytext[yyleng-1] = '\0'; yylval->id = string_alloc(yytext); return DS_FUN; }

"imm"        { return DS_IMM; }

"+"          { OP(add); }
"-"          { OP(sub); }
"*"          { IBINOP(mul); }
"/"          { IBINOP(div); }
"%"          { IBINOP(mod); }

"=="         { BINOP(eq); }
"!="         { BINOP(ne); }
"<"          { BINOP(lt); }
"<="         { BINOP(le); }
">"          { BINOP(gt); }
">="         { BINOP(ge); }

"&&"         { BINOP(land); }
"||"         { BINOP(lor);  }

"&"          { BINOP(band); }
"|"          { BINOP(bor);  }
"^"          { BINOP(bxor); }
"<<"         { BINOP(blshift); }
">>"         { BINOP(brshift);   }

"++"         { UNOP(inc); }
"--"         { UNOP(dec); }
"="          { UNOP(mov); }
"!"          { UNOP(not); }
"~"          { UNOP(cmp); }

"call"       { return DS_CALL; }
"return"     { return DS_RETURN; }

"jump"       { _JUMP(); }
"jump=="     { JUMP(eq); }
"jump!="     { JUMP(ne); }
"jump<"      { JUMP(lt); }
"jump<="     { JUMP(le); }
"jump>"      { JUMP(gt); }
"jump>="     { JUMP(ge); }

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
