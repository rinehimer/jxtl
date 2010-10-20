SUBDIRS = libjxtl \
          src \
          test

bindings: swig-perl

PERL_BINDINGS_DIR=${top_srcdir}/bindings/perl

swig-perl:
	${SWIG} -I${top_srcdir}/libjxtl -I${top_srcdir}/bindings/include -perl5 ${PERL_BINDINGS_DIR}/libjxtl.i
	cd ${PERL_BINDINGS_DIR} && perl Makefile.PL
	${MAKE} -C ${PERL_BINDINGS_DIR}

install-swig-perl:
	${MAKE} -C ${PERL_BINDINGS_DIR} install
