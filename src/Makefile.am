INCLUDES = -I${top_srcdir}/libjxtl
bin_PROGRAMS = jxtl xml2json jxtl_path_convert
jxtl_SOURCES = jxtl.c jxtl_parse.y jxtl_lex.l
xml2json_SOURCES = xml2json.c
jxtl_path_convert_SOURCES = jxtl_path_convert.l

LIBJXTL = ${top_srcdir}/libjxtl/libjxtl.la

YFLAGS =
jxtl_CFLAGS = -g ${APR_CFLAGS} ${XML2_CFLAGS}
jxtl_LDFLAGS = ${APR_LIBS} ${XML2_LIBS} -Wl,-rpath -Wl,${libdir}
jxtl_LDADD = ${LIBJXTL}

xml2json_CFLAGS = -g ${APR_CFLAGS} ${XML2_CFLAGS}
xml2json_LDFLAGS = ${APR_LIBS} ${XML2_LIBS} -Wl,-rpath -Wl,${libdir}
xml2json_LDADD = ${LIBJXTL}

jxtl_path_convert_CFLAGS = -g

${jxtl_OBJECTS}: jxtl_parse.h jxtl_parse.c jxtl_lex.h jxtl_lex.c

jxtl_parse.c jxtl_parse.h: jxtl_parse.y
jxtl_lex.c jxtl_lex.h: jxtl_lex.l
jxtl_path_convert.c: jxtl_path_convert.l

%.c %.h: %.l
	${LEXCOMPILE} $<

.l.c:
	${LEXCOMPILE} $<
