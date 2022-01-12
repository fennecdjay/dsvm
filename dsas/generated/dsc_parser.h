/* A Bison parser, made by GNU Bison 3.7.2.51-de63.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_DSAS_GENERATED_DSC_PARSER_H_INCLUDED
# define YY_DSAS_GENERATED_DSC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef DSASDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define DSASDEBUG 1
#  else
#   define DSASDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define DSASDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined DSASDEBUG */
#if DSASDEBUG
extern int dsasdebug;
#endif

/* Token kinds.  */
#ifndef DSASTOKENTYPE
# define DSASTOKENTYPE
  enum dsastokentype
  {
    DSASEMPTY = -2,
    DSASEOF = 0,                   /* "end of file"  */
    DSASerror = 256,               /* error  */
    DSASUNDEF = 257,               /* "invalid token"  */
    DS_IMM = 258,                  /* "imm"  */
    DS_CALL = 259,                 /* "call"  */
    DS_RETURN = 260,               /* "return"  */
    DS_IF = 261,                   /* "if"  */
    DS_GOTO = 262,                 /* "goto"  */
    DS_BINOP = 263,                /* "<binop>"  */
    DS_UNOP = 264,                 /* "<unop>"  */
    DS_OP = 265,                   /* "<op>"  */
    DS_NUM = 266,                  /* "<integer>"  */
    DS_REG = 267,                  /* "<register>"  */
    DS_LABEL = 268,                /* "<label>"  */
    DS_FNUM = 269,                 /* "<float>"  */
    DS_ID = 270,                   /* "<id>"  */
    DS_FUN = 271                   /* "<function>"  */
  };
  typedef enum dsastokentype dsastoken_kind_t;
#endif

/* Value type.  */
#if ! defined DSASSTYPE && ! defined DSASSTYPE_IS_DECLARED
union DSASSTYPE
{
#line 22 "dsas.y"

  char     *id;
  uintptr_t num;
  float     fnum;
  ds_opcode op;

#line 95 "generated/dsc_parser.h"

};
typedef union DSASSTYPE DSASSTYPE;
# define DSASSTYPE_IS_TRIVIAL 1
# define DSASSTYPE_IS_DECLARED 1
#endif



int dsasparse (DsScanner* arg);

#endif /* !YY_DSAS_GENERATED_DSC_PARSER_H_INCLUDED  */
