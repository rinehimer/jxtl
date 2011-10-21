/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1

/* Substitute the variable and function names.  */
#define yyparse         jxtl_parse
#define yylex           jxtl_lex
#define yyerror         jxtl_error
#define yylval          jxtl_lval
#define yychar          jxtl_char
#define yydebug         jxtl_debug
#define yynerrs         jxtl_nerrs
#define yylloc          jxtl_lloc

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 22 "jxtl_parse.y"

#include <stdarg.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

/*
 * Define YY_DECL before including jxtl_lex.h so that it knows we are doing a
 * custom declaration of jxtl_lex.
 */
#define YY_DECL

#include "apr_macros.h"
#include "json.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "jxtl.h"
#include "parser.h"

#define callbacks ((jxtl_callback_t *) callbacks_ptr)

int jxtl_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
              yyscan_t yyscanner );
void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );


/* Line 189 of yacc.c  */
#line 108 "jxtl_parse.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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
     T_FORMAT = 262,
     T_END = 263,
     T_IF = 264,
     T_ELSEIF = 265,
     T_ELSE = 266,
     T_TEXT = 267,
     T_PATH_EXPR = 268,
     T_STRING = 269
   };
#endif
/* Tokens.  */
#define T_DIRECTIVE_START 258
#define T_DIRECTIVE_END 259
#define T_SECTION 260
#define T_SEPARATOR 261
#define T_FORMAT 262
#define T_END 263
#define T_IF 264
#define T_ELSEIF 265
#define T_ELSE 266
#define T_TEXT 267
#define T_PATH_EXPR 268
#define T_STRING 269




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 61 "jxtl_parse.y"

  int ival;
  unsigned char *string;



/* Line 214 of yacc.c  */
#line 179 "jxtl_parse.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 204 "jxtl_parse.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   53

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  17
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  16
/* YYNRULES -- Number of rules.  */
#define YYNRULES  29
/* YYNRULES -- Number of states.  */
#define YYNSTATES  57

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    15,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    16,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    15,    18,    19,
      25,    26,    37,    38,    46,    47,    55,    56,    63,    65,
      69,    70,    73,    76,    79,    82,    83,    85,    89,    93
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      18,     0,    -1,    19,    -1,    -1,    19,    12,    -1,    19,
      20,    -1,    19,    22,    -1,    19,    24,    -1,    -1,     3,
      13,    21,    31,     4,    -1,    -1,     3,     5,    13,    23,
      31,     4,    30,     3,     8,     4,    -1,    -1,     3,     9,
      13,     4,    25,    30,    26,    -1,    -1,     3,    10,    13,
       4,    27,    30,    26,    -1,    -1,     3,    11,     4,    28,
      30,    29,    -1,    29,    -1,     3,     8,     4,    -1,    -1,
      30,    12,    -1,    30,    20,    -1,    30,    22,    -1,    30,
      24,    -1,    -1,    32,    -1,    31,    15,    32,    -1,     6,
      16,    14,    -1,     7,    16,    14,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    76,    76,    79,    81,    87,    88,    89,    94,    93,
     106,   105,   125,   124,   139,   138,   148,   147,   154,   158,
     165,   167,   173,   174,   175,   178,   180,   181,   185,   197
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\"{{\"", "\"}}\"", "\"section\"",
  "\"separator\"", "\"format\"", "\"end\"", "\"if\"", "\"elseif\"",
  "\"else\"", "\"text\"", "\"path expression\"", "\"string\"", "','",
  "'='", "$accept", "document", "text", "value_directive", "$@1",
  "section_directive", "$@2", "if_directive", "$@3", "rest_of_if", "$@4",
  "$@5", "endif", "section_content", "options", "option", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    44,    61
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    17,    18,    19,    19,    19,    19,    19,    21,    20,
      23,    22,    25,    24,    27,    26,    28,    26,    26,    29,
      30,    30,    30,    30,    30,    31,    31,    31,    32,    32
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     2,     2,     0,     5,
       0,    10,     0,     7,     0,     7,     0,     6,     1,     3,
       0,     2,     2,     2,     2,     0,     1,     3,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     2,     1,     0,     4,     5,     6,     7,     0,
       0,     8,    10,     0,    25,    25,    12,     0,     0,     0,
      26,     0,    20,     0,     0,     9,     0,    20,     0,    28,
      29,    27,     0,     0,    21,    22,    23,    24,    13,    18,
       0,     0,     0,     0,     0,    19,     0,    16,    11,    14,
      20,    20,     0,     0,     0,    17,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    35,    14,    36,    15,    37,    22,    38,
      51,    50,    39,    28,    19,    20
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -28
static const yytype_int8 yypact[] =
{
     -28,     9,     2,   -28,    31,   -28,   -28,   -28,   -28,   -10,
      -9,   -28,   -28,     6,    12,    12,   -28,    -5,    22,    -3,
     -28,    -2,   -28,     7,    27,   -28,    12,   -28,     3,   -28,
     -28,   -28,     4,    17,   -28,   -28,   -28,   -28,   -28,   -28,
      24,    16,    29,    39,    41,   -28,    42,   -28,   -28,   -28,
     -28,   -28,     5,     3,    26,   -28,   -28
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -28,   -28,   -28,    45,   -28,    46,   -28,    47,   -28,    -1,
     -28,   -28,     1,   -27,    35,    25
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      32,    25,    27,    12,    13,     4,    33,    40,    54,     3,
      16,    23,    26,    26,     5,    34,    34,    34,    17,    18,
      45,    29,     9,    52,    53,    41,    10,    42,    43,     9,
      11,     9,    44,    10,    41,    10,     9,    11,    24,    11,
      10,    30,    46,    47,    11,    48,    49,     6,     7,     8,
      21,    31,    56,    55
};

static const yytype_uint8 yycheck[] =
{
      27,     4,     4,    13,    13,     3,     3,     3,     3,     0,
       4,    16,    15,    15,    12,    12,    12,    12,     6,     7,
       4,    14,     5,    50,    51,     8,     9,    10,    11,     5,
      13,     5,     8,     9,     8,     9,     5,    13,    16,    13,
       9,    14,    13,     4,    13,     4,     4,     2,     2,     2,
      15,    26,    53,    52
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    18,    19,     0,     3,    12,    20,    22,    24,     5,
       9,    13,    13,    13,    21,    23,     4,     6,     7,    31,
      32,    31,    25,    16,    16,     4,    15,     4,    30,    14,
      14,    32,    30,     3,    12,    20,    22,    24,    26,    29,
       3,     8,    10,    11,     8,     4,    13,     4,     4,     4,
      28,    27,    30,    30,     3,    29,    26
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, scanner, parser, callbacks_ptr, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, scanner)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, scanner, parser, callbacks_ptr); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, yyscan_t scanner, parser_t *parser, void *callbacks_ptr)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, parser, callbacks_ptr)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    yyscan_t scanner;
    parser_t *parser;
    void *callbacks_ptr;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (parser);
  YYUSE (callbacks_ptr);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, yyscan_t scanner, parser_t *parser, void *callbacks_ptr)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, parser, callbacks_ptr)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    yyscan_t scanner;
    parser_t *parser;
    void *callbacks_ptr;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, scanner, parser, callbacks_ptr);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, yyscan_t scanner, parser_t *parser, void *callbacks_ptr)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, scanner, parser, callbacks_ptr)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    yyscan_t scanner;
    parser_t *parser;
    void *callbacks_ptr;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , scanner, parser, callbacks_ptr);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, scanner, parser, callbacks_ptr); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, yyscan_t scanner, parser_t *parser, void *callbacks_ptr)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, scanner, parser, callbacks_ptr)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    yyscan_t scanner;
    parser_t *parser;
    void *callbacks_ptr;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (scanner);
  YYUSE (parser);
  YYUSE (callbacks_ptr);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (yyscan_t scanner, parser_t *parser, void *callbacks_ptr);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (yyscan_t scanner, parser_t *parser, void *callbacks_ptr)
#else
int
yyparse (scanner, parser, callbacks_ptr)
    yyscan_t scanner;
    parser_t *parser;
    void *callbacks_ptr;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:

/* Line 1464 of yacc.c  */
#line 82 "jxtl_parse.y"
    {
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, (yyvsp[(2) - (2)].string) );
      }
    }
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 94 "jxtl_parse.y"
    {
      if ( callbacks->value_handler &&
           !callbacks->value_handler( callbacks->user_data, (yyvsp[(2) - (2)].string) ) ) {
        jxtl_error( &(yylsp[(2) - (2)]), scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 106 "jxtl_parse.y"
    {
      if ( callbacks->section_start_handler &&
          !callbacks->section_start_handler( callbacks->user_data,
                                              (yyvsp[(3) - (3)].string) ) ) {
        jxtl_error( &(yylsp[(2) - (3)]), scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 116 "jxtl_parse.y"
    { 
      if ( callbacks->section_end_handler ) {
        callbacks->section_end_handler( callbacks->user_data );
      }
    }
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 125 "jxtl_parse.y"
    {
      if ( callbacks->if_start_handler &&
           !callbacks->if_start_handler( callbacks->user_data, (yyvsp[(3) - (4)].string) ) ) {
        jxtl_error( &(yylsp[(3) - (4)]), scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 139 "jxtl_parse.y"
    {
      if ( callbacks->elseif_handler &&
           !callbacks->elseif_handler( callbacks->user_data, (yyvsp[(3) - (4)].string) ) ) {
        jxtl_error( &(yylsp[(3) - (4)]), scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 148 "jxtl_parse.y"
    {
      if ( callbacks->else_handler ) {
        callbacks->else_handler( callbacks->user_data );
      }
    }
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 159 "jxtl_parse.y"
    {
      if ( callbacks->if_end_handler ) {
        callbacks->if_end_handler( callbacks->user_data );
      }
    }
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 168 "jxtl_parse.y"
    {
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, (yyvsp[(2) - (2)].string) );
      }
    }
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 186 "jxtl_parse.y"
    {
      if ( callbacks->separator_start_handler ) {
        callbacks->separator_start_handler( callbacks->user_data );
      }
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, (yyvsp[(3) - (3)].string) );
      }
      if ( callbacks->separator_end_handler ) {
        callbacks->separator_end_handler( callbacks->user_data );
      }
    }
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 198 "jxtl_parse.y"
    {
      if ( callbacks->format_handler ) {
        callbacks->format_handler( callbacks->user_data, (yyvsp[(3) - (3)].string) );
      }
    }
    break;



/* Line 1464 of yacc.c  */
#line 1632 "jxtl_parse.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, scanner, parser, callbacks_ptr, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (&yylloc, scanner, parser, callbacks_ptr, yymsg);
	  }
	else
	  {
	    yyerror (&yylloc, scanner, parser, callbacks_ptr, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, scanner, parser, callbacks_ptr);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, scanner, parser, callbacks_ptr);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, scanner, parser, callbacks_ptr, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc, scanner, parser, callbacks_ptr);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, scanner, parser, callbacks_ptr);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 205 "jxtl_parse.y"


/**
 * Structure to hold data during parsing.  One of these will be passed to the
 * callback functions.
 */
typedef struct jxtl_data_t {
  /** Memory pool */
  apr_pool_t *mp;
  /** Pointer to the JSON object */
  json_t *json;
  /** Pointer to the current content array. */
  apr_array_header_t *current_array;
  /** Array of content arrays. */
  apr_array_header_t *content_array;
  /** Pointer to the last section or value pushed on. */
  jxtl_content_t *last_section_or_value;
  /** Reusable parser. */
  parser_t *jxtl_path_parser;
} jxtl_data_t;

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "%s: %d.%d-%d.%d ", parser->get_filename( parser ),
           yylloc->first_line, yylloc->first_column, yylloc->last_line,
           yylloc->last_column );
  va_start( args, error_string );
  vfprintf( stderr, error_string, args );
  fprintf( stderr, "\n" );
  va_end( args );
}

/**
 * Allocate a new jxtl_data_t.
 */
static jxtl_data_t *jxtl_data_create( apr_pool_t *mp )
{
  jxtl_data_t *data = apr_palloc( mp, sizeof(jxtl_data_t) );

  data->mp = mp;
  data->json = NULL;
  data->content_array = apr_array_make( data->mp, 1024,
                                        sizeof(apr_array_header_t *) );
  data->current_array = NULL;
  data->jxtl_path_parser = jxtl_path_parser_create( mp );

  return data;
}

/**
 * Reset the data each time before parsing.  Basically we have to clear out
 * the content_array and create a new initial array.  Note that the memory
 * pool used is not cleared because it's not owned by us and clearing it could
 * clear the allocation of a previous template that was parsed using the same
 * parser.
 */
static void jxtl_data_reset( jxtl_data_t *data )
{
  apr_array_header_t *initial_array;
  APR_ARRAY_CLEAR( data->content_array );
  initial_array = apr_array_make( data->mp, 1024,
                                  sizeof(apr_array_header_t *) );
  APR_ARRAY_PUSH( data->content_array, apr_array_header_t * ) = initial_array;
  data->current_array = initial_array;
}

/*
 * Convenience function to create a new content object and it on the current
 * array.
 */
static void jxtl_content_push( jxtl_data_t *data, jxtl_content_type type,
                               void *value )
{
  jxtl_content_t *content = NULL;

  content = apr_palloc( data->mp, sizeof(jxtl_content_t) );
  content->type = type;
  content->value = value;
  content->separator = NULL;
  content->format = NULL;

  if ( ( type == JXTL_SECTION ) || ( type == JXTL_VALUE ) ) {
    data->last_section_or_value = content;
  }

  APR_ARRAY_PUSH( data->current_array, jxtl_content_t * ) = content;
}

/*****************************************************************************
 * Parser callbacks
 *****************************************************************************/

/**
 * Parser callback for when it finds text.
 * @param user_data The jxtl_data.
 * @param text The text.
 */
static void jxtl_text_func( void *user_data, unsigned char *text )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_content_push( data, JXTL_TEXT, apr_pstrdup( data->mp, (char *) text ) );
}

/**
 * Parser callback for starting a section.  We need to allocate a new section,
 * parse the expression and then add it to our content array.  We also need to
 * make sure the current_array points to the one in the new section object so 
 * that content encountered after the section is properly added to it.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the section
 */
static int jxtl_section_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;
  int result;

  section = apr_palloc( data->mp, sizeof(jxtl_section_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &section->expr );
  section->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  jxtl_content_push( data, JXTL_SECTION, section );
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = section->content;

  return result;
}

/**
 * Parser callback for when a section ends.  All we need to do is reset the
 * current_array to the previous item on the content_array stack.
 * @param user_data The jxtl_data.
 */
static void jxtl_section_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback for starting an if statement.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the if
 */
static int jxtl_if_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;
  int result;

  if_block = apr_array_make( data->mp, 8, sizeof(jxtl_if_t *) );
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  jxtl_content_push( data, JXTL_IF, if_block );

  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = jxtl_if->content;

  return result;
}

/**
 * Parser callback for starting an elseif statement.  A little different than
 * starting an if because we need to get the if_block array and add the new
 * jxtl_if_t object to that.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the elseif
 */
static int jxtl_elseif( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;
  int result;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;

  return result;
}

static void jxtl_else( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;

  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  jxtl_if->expr = NULL;
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;
}

static void jxtl_if_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback for when it encounters a separator directive.  All this does
 * is take the current section and set its current_array to the separator.
 * @param user_data The jxtl_data.
 */
static void jxtl_separator_start( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_content_t *content = data->last_section_or_value;

  content->separator = apr_array_make( data->mp, 1, sizeof(jxtl_content_t *) );

  /*
   * Save off the current array and then make the current array the separator.
   */
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = content->separator;
}

/**
 * Parser callback for when a separator directive is ended.  Just sets the
 * current_array of the section back to the content.
 * @param user_data The jxtl_data.
 */
static void jxtl_separator_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback function for when it encounters a value reference in the
 * template, i.e. {{value}}.  If we are not nested at all, it is printed
 * immediately.  Otherwise, the name is just saved off for later processing.
 * @param user_data The jxtl_data.
 * @param name The name of the value to lookup.
 */
static int jxtl_value_func( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_path_expr_t *path_expr;
  int result;

  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &path_expr );
  jxtl_content_push( data, JXTL_VALUE, path_expr );

  return result;
}

static char *jxtl_get_error( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  return parser_get_error( data->jxtl_path_parser );
}

static void jxtl_format( void *user_data, char *format )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  data->last_section_or_value->format = apr_pstrdup( data->mp, format );
}

static jxtl_template_t *jxtl_template_create( apr_pool_t *mp,
                                              apr_array_header_t *content )
{
  jxtl_template_t *template;
  template = apr_palloc( mp, sizeof(jxtl_template_t) );
  template->content = content;
  template->flush_func = NULL;
  template->flush_data = NULL;
  template->formats = apr_hash_make( mp );
  template->format_data = NULL;

  return template;
}

parser_t *jxtl_parser_create( apr_pool_t *mp, jxtl_callback_t *user_callbacks )
{
  jxtl_callback_t *callbacks_ptr;
  parser_t *parser = parser_create( mp,
                                    jxtl_lex_init,
                                    jxtl_set_extra,
                                    jxtl_lex_destroy,
                                    jxtl__scan_buffer,
                                    jxtl__delete_buffer,
                                    jxtl_parse );
  if ( user_callbacks ) {
    callbacks_ptr = user_callbacks;
    callbacks_ptr->own_user_data = FALSE;
  }
  else {
    callbacks_ptr = apr_palloc( mp, sizeof(jxtl_callback_t) );
    callbacks_ptr->text_handler = jxtl_text_func;
    callbacks_ptr->section_start_handler = jxtl_section_start;
    callbacks_ptr->section_end_handler = jxtl_section_end;
    callbacks_ptr->if_start_handler = jxtl_if_start;
    callbacks_ptr->elseif_handler = jxtl_elseif;
    callbacks_ptr->else_handler = jxtl_else;
    callbacks_ptr->if_end_handler = jxtl_if_end;
    callbacks_ptr->separator_start_handler = jxtl_separator_start;
    callbacks_ptr->separator_end_handler = jxtl_separator_end;
    callbacks_ptr->value_handler = jxtl_value_func;
    callbacks_ptr->get_error_func = jxtl_get_error;
    callbacks_ptr->format_handler = jxtl_format;
    callbacks_ptr->own_user_data = TRUE;
    callbacks_ptr->user_data = jxtl_data_create( mp );
  }

  parser_set_user_data( parser, callbacks_ptr );

  return parser;
}

int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            jxtl_template_t **template_ptr )
{
  jxtl_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  jxtl_data_t *jxtl_data = (jxtl_data_t *) jxtl_callbacks->user_data;
  int result = FALSE;

  *template_ptr = NULL;
  if ( jxtl_callbacks->own_user_data ) {
    jxtl_data_reset( jxtl_data );
  }

  if ( parser_parse_file( parser, file ) ) {
    if ( jxtl_callbacks->own_user_data ) {
      *template_ptr = jxtl_template_create( parser->mp,
                                            jxtl_data->current_array );
    }
    result = TRUE;
  }

  return result;
}

int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
                              jxtl_template_t **template_ptr )
{
  jxtl_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  jxtl_data_t *jxtl_data = (jxtl_data_t *) jxtl_callbacks->user_data;
  int result = FALSE;

  *template_ptr = NULL;
  if ( jxtl_callbacks->own_user_data ) {
    jxtl_data_reset( jxtl_data );
  }

  if ( parser_parse_buffer( parser, buffer ) ) {
    if ( jxtl_callbacks->own_user_data ) {
      *template_ptr = jxtl_template_create( parser->mp,
                                            jxtl_data->current_array );
    }
    result = TRUE;
  }

  return result;
}

