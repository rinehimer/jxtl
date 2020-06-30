SUBDIRS = libjxtl \
          src \
          test

ACLOCAL_AMFLAGS = -I m4

bindings: swig-perl swig-python
install-bindings: install-swig-perl install-swig-python

PERL_BINDINGS_DIR=${top_srcdir}/bindings/perl
PYTHON_BINDINGS_DIR=${top_srcdir}/bindings/python

swig-perl:
	${SWIG} -I${top_srcdir}/libjxtl -I${top_srcdir}/bindings/include -perl5 ${PERL_BINDINGS_DIR}/libjxtl.i
	cd ${PERL_BINDINGS_DIR} && perl Makefile.PL
	${MAKE} -C ${PERL_BINDINGS_DIR}

install-swig-perl:
	${MAKE} -C ${PERL_BINDINGS_DIR} install

swig-python:
	${SWIG} -I${top_srcdir}/libjxtl -I${top_srcdir}/bindings/include -python ${PYTHON_BINDINGS_DIR}/libjxtl.i
	cd ${PYTHON_BINDINGS_DIR} && python3 setup.py build_ext --inplace

install-swig-python:
	cd ${PYTHON_BINDINGS_DIR} && python3 setup.py install
