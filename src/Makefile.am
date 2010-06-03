INCLUDES = -I${top_srcdir}/libjxtl
bin_PROGRAMS = jxtl
jxtl_SOURCES = jxtl.c jxtl_parse.y jxtl_lex.l

LIBJXTL = ${top_srcdir}/libjxtl/libjxtl.la

YFLAGS =
jxtl_CFLAGS = -g ${APR_CFLAGS} ${XML2_CFLAGS}
jxtl_LDFLAGS = ${APR_LIBS} ${XML2_LIBS}
jxtl_LDADD = ${LIBJXTL}

${jxtl_OBJECTS}: jxtl_parse.h jxtl_parse.c jxtl_lex.h jxtl_lex.c

jxtl_parse.c jxtl_parse.h: jxtl_parse.y
jxtl_lex.c jxtl_lex.h: jxtl_lex.l

%.c %.h: %.l
	${LEXCOMPILE} $<

.l.c:
	${LEXCOMPILE} $<
