/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_DIRECTIVE_START = 258,
     T_DIRECTIVE_END = 259,
     T_SECTION = 260,
     T_SEPARATOR = 261,
     T_TEST = 262,
     T_END = 263,
     T_IF = 264,
     T_ELSE = 265,
     T_TEXT = 266,
     T_IDENTIFIER = 267,
     T_STRING = 268
   };
#endif
#define T_DIRECTIVE_START 258
#define T_DIRECTIVE_END 259
#define T_SECTION 260
#define T_SEPARATOR 261
#define T_TEST 262
#define T_END 263
#define T_IF 264
#define T_ELSE 265
#define T_TEXT 266
#define T_IDENTIFIER 267
#define T_STRING 268




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 49 "jxtl_parse.y"
typedef union YYSTYPE {
  int ival;
  unsigned char *string;
} YYSTYPE;
/* Line 1274 of yacc.c.  */
#line 68 "jxtl_parse.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




