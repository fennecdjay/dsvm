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

#ifndef YY_DS_GENERATED_DSC_PARSER_H_INCLUDED
# define YY_DS_GENERATED_DSC_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef DSDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define DSDEBUG 1
#  else
#   define DSDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define DSDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined DSDEBUG */
#if DSDEBUG
extern int dsdebug;
#endif

/* Token kinds.  */
#ifndef DSTOKENTYPE
# define DSTOKENTYPE
  enum dstokentype
  {
    DSEMPTY = -2,
    DSEOF = 0,                     /* "end of file"  */
    DSerror = 256,                 /* error  */
    DSUNDEF = 257,                 /* "invalid token"  */
    DS_IMM = 258,                  /* "imm"  */
    DS_CALL = 259,                 /* "call"  */
    DS_RETURN = 260,               /* "return"  */
    DS_BINOP = 261,                /* "<binop>"  */
    DS_IBINOP = 262,               /* "<ibinop>"  */
    DS_JUMP = 263,                 /* "<jump>"  */
    DS_UNOP = 264,                 /* "<unop>"  */
    DS_OP = 265,                   /* "<op>"  */
    DS_NUM = 266,                  /* "<integer>"  */
    DS_REG = 267,                  /* "<register>"  */
    DS_LABEL = 268,                /* "<label>"  */
    DS_FNUM = 269,                 /* "<float>"  */
    DS_ID = 270,                   /* "<id>"  */
    DS_FUN = 271                   /* "<function>"  */
  };
  typedef enum dstokentype dstoken_kind_t;
#endif

/* Value type.  */
#if ! defined DSSTYPE && ! defined DSSTYPE_IS_DECLARED
union DSSTYPE
{
#line 26 "dsc.y"

  char     *id;
  uintptr_t num;
  float     fnum;
  ds_opcode op;

#line 95 "generated/dsc_parser.h"

};
typedef union DSSTYPE DSSTYPE;
# define DSSTYPE_IS_TRIVIAL 1
# define DSSTYPE_IS_DECLARED 1
#endif



int dsparse (DsScanner* arg);

#endif /* !YY_DS_GENERATED_DSC_PARSER_H_INCLUDED  */
