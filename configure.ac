#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(jxtl, 0.1.0, rinehimer@gmail.com)
AC_CONFIG_SRCDIR([libjxtl/utf.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX

# Checks for libraries.
AC_CHECK_LIB([fl], [yywrap])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strcasecmp strdup])

## Apache portable runtime checking
AC_CHECK_PROG(APR_CONFIG, apr-1-config, apr-1-config, [], [], [])
AC_CHECK_PROG(APR_CONFIG, apr-config, apr-config, [], [], [exit1])
APR_LIBS=`$APR_CONFIG --link-ld`
AC_SUBST(APR_LIBS)
APR_CFLAGS="`$APR_CONFIG --includes` `$APR_CONFIG --cflags` `$APR_CONFIG --cppflags`"
AC_SUBST(APR_CFLAGS)

AC_CHECK_PROG(XML2_CONFIG, xml2-config, xml2-config, [], [], [exit1])
XML2_CFLAGS=`$XML2_CONFIG --cflags`
AC_SUBST(XML2_CFLAGS)
XML2_LIBS=`$XML2_CONFIG --libs`
AC_SUBST(XML2_LIBS)

AC_CONFIG_FILES([Makefile
                 libjxtl/Makefile
		 src/Makefile])
AC_OUTPUT
