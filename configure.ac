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
AC_CHECK_PROGS(APR_CONFIG, apr-1-config apr-config, [/bin/false],
               [$PATH$PATH_SEPARATOR/usr/local/apache2/bin$PATH_SEPARATOR]dnl
               [/usr/apache2/bin])
if test "x$APR_CONFIG" = "x/bin/false" ; then
  AC_MSG_ERROR(Could not find an apr-config, [1])
fi
APR_CFLAGS=`$APR_CONFIG --cflags --cppflags --includes`
APR_LIBS=`$APR_CONFIG --link-ld --libs`
AC_SUBST(APR_CFLAGS)
AC_SUBST(APR_LIBS)

AC_CHECK_PROGS(APU_CONFIG, apu-1-config apu-config, [/bin/false],
               [$PATH$PATH_SEPARATOR/usr/local/apache2/bin$PATH_SEPARATOR]dnl
               [/usr/apache2/bin])
if test "x$APU_CONFIG" = "x/bin/false" ; then
  AC_MSG_ERROR(Could not find an apu-config, [1])
fi

## Don't bother linking against dbm if our apu-config supports it.
if $APU_CONFIG --avoid-dbm > /dev/null 2>&1 ; then
  DBM_FLAGS="--avoid-dbm"
fi

APU_CFLAGS=`$APU_CONFIG --includes`
APU_LIBS=`$APU_CONFIG $DBM_FLAGS --link-ld --libs`
AC_SUBST(APU_CFLAGS)
AC_SUBST(APU_LIBS)

AC_CHECK_PROG(SWIG, swig, swig, [], [], [])

AC_CONFIG_FILES([Makefile
                 bindings/perl/Makefile.PL
                 bindings/python/setup.py
                 libjxtl/Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT
