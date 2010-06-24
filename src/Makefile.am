INCLUDES = -I${top_srcdir}/libjxtl
bin_PROGRAMS = jxtl xml2json json_path
jxtl_SOURCES = jxtl.c jxtl_parse.y jxtl_lex.l
xml2json_SOURCES = xml2json.c
json_path_SROUCES = json_path.c

LIBJXTL = ${top_srcdir}/libjxtl/libjxtl.la

YFLAGS =
jxtl_CFLAGS = -g ${APR_CFLAGS} ${XML2_CFLAGS}
jxtl_LDFLAGS = ${APR_LIBS} ${XML2_LIBS} -Wl,-rpath -Wl,${libdir}
jxtl_LDADD = ${LIBJXTL}

xml2json_CFLAGS = -g ${APR_CFLAGS} ${XML2_CFLAGS}
xml2json_LDFLAGS = ${APR_LIBS} ${XML2_LIBS} -Wl,-rpath -Wl,${libdir}
xml2json_LDADD = ${LIBJXTL}

json_path_CFLAGS = -g ${APR_CFLAGS}
json_path_LDFLAGS = ${APR_LIBS} -Wl,-rpath -Wl,${libdir}
json_path_LDADD = ${LIBJXTL}

${jxtl_OBJECTS}: jxtl_parse.h jxtl_parse.c jxtl_lex.h jxtl_lex.c

jxtl_parse.c jxtl_parse.h: jxtl_parse.y
jxtl_lex.c jxtl_lex.h: jxtl_lex.l

%.c %.h: %.l
	${LEXCOMPILE} $<

.l.c:
	${LEXCOMPILE} $<
