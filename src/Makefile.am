INCLUDES = -I${top_srcdir}/libjxtl
bin_PROGRAMS = jxtl xml2json
jxtl_SOURCES = jxtl.c jxtl_parse.y jxtl_lex.l
xml2json_SOURCES = xml2json.c

LIBJXTL = ${top_srcdir}/libjxtl/libjxtl.la

YFLAGS =
jxtl_CFLAGS = -g ${APR_CFLAGS} ${APU_CFLAGS}
jxtl_LDFLAGS = ${APR_LIBS} ${APU_LIBS} -Wl,-rpath -Wl,${libdir}
jxtl_LDADD = ${LIBJXTL}

xml2json_CFLAGS = -g ${APR_CFLAGS} ${APU_CFLAGS}
xml2json_LDFLAGS = ${APR_LIBS} ${APU_LIBS} -Wl,-rpath -Wl,${libdir}
xml2json_LDADD = ${LIBJXTL}

${jxtl_OBJECTS}: jxtl_parse.h jxtl_parse.c jxtl_lex.h jxtl_lex.c

jxtl_parse.c jxtl_parse.h: jxtl_parse.y
jxtl_lex.c jxtl_lex.h: jxtl_lex.l

%.c %.h: %.l
	${LEXCOMPILE} $<

.l.c:
	${LEXCOMPILE} $<
