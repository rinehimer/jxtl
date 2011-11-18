lib_LTLIBRARIES = libjxtl.la

libjxtlincdir = $(includedir)/libjxtl

libjxtlinc_HEADERS = apr_macros.h \
                     json.h \
                     jxtl.h \
                     jxtl_path.h \
                     jxtl_template.h \
                     json_writer.h \
                     json_writer_ctx.h \
                     parser.h \
                     str_buf.h \
                     utf.h \
                     xml2json.h

libjxtl_la_SOURCES = json.c \
                     json_lex.l \
                     json_parse.y \
                     jxtl_lex.l \
                     jxtl_parse.y \
                     jxtl_path.c \
                     jxtl_path_lex.l \
                     jxtl_path_parse.y \
                     jxtl_template.c \
                     json_writer.c \
                     json_writer_ctx.c \
                     misc.c \
                     misc.h \
                     parser.c \
                     parser_lex_macros.h \
                     str_buf.c \
                     utf.c \
                     xml2json.c

$(libjxtl_la_OBJECTS): json_parse.c json_parse.h json_lex.c json_lex.h \
                       jxtl_parse.c jxtl_parse.h jxtl_lex.c jxtl_lex.h \
                       jxtl_path_parse.c jxtl_path_parse.h jxtl_path_lex.c \
                       jxtl_path_lex.h

libjxtl_la_CFLAGS = ${APR_CFLAGS} ${APU_CFLAGS}
libjxtl_la_LDFLAGS = ${APR_LIBS} ${APU_LIBS}

YFLAGS =

%_lex.c %_lex.h: %_lex.l
%_parse.c %_parse.h: %_parse.y

%.c %.h: %.l
	$(LEXCOMPILE) $<

.l.c:
	$(LEXCOMPILE) $<
