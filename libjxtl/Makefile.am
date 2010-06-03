lib_LTLIBRARIES = libjxtl.la

libjxtlincdir = $(includedir)/libjxtl

libjxtlinc_HEADERS = apr_macros.h \
                     json.h \
                     json_node.h \
                     json_parser.h \
                     json_writer.h \
                     lex_extra.h \
                     utf.h \
                     xml2json.h

libjxtl_la_SOURCES = json.c \
                     json_lex.l \
                     json_node.c \
                     json_parse.y \
                     json_parser.c \
                     json_writer.c \
                     lex_extra.c \
                     utf.c \
                     xml2json.c

$(libjxtl_la_OBJECTS): json_parse.c json_parse.h json_lex.c json_lex.h

libjxtl_la_CFLAGS = ${APR_CFLAGS} ${XML2_CFLAGS}
libjxtl_la_LDFLAGS = ${APR_LIBS} ${XML2_LIBS}

json_parse.c json_parse.h: json_parse.y
json_lex.c json_lex.h: json_lex.l

%.c %.h: %.l
	$(LEXCOMPILE) $<

.l.c:
	$(LEXCOMPILE) $<
