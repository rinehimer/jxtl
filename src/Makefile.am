AM_CPPFLAGS = -I${top_srcdir}/libjxtl
bin_PROGRAMS = jxtl xml2json
if BUILD_STATIC
bin_PROGRAMS += xml2json_static
endif
jxtl_SOURCES = jxtl.c
xml2json_SOURCES = xml2json.c
xml2json_static_SOURCES = $(xml2json_SOURCES)

LIBJXTL = ${top_srcdir}/libjxtl/libjxtl.la

jxtl_CFLAGS = -g ${APR_CFLAGS} ${APU_CFLAGS}
jxtl_LDFLAGS = ${APR_LIBS} ${APU_LIBS}
jxtl_LDADD = ${LIBJXTL}

xml2json_CFLAGS = -g ${APR_CFLAGS} ${APU_CFLAGS}
xml2json_LDFLAGS = ${APR_LIBS} ${APU_LIBS}
xml2json_LDADD = ${LIBJXTL}

xml2json_static_CFLAGS = -static $(xml2json_CFLAGS)
xml2json_static_LDFLAGS = -static $(xml2json_LDFLAGS)
xml2json_static_LDADD = ${LIBJXTL}
