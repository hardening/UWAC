lib_LTLIBRARIES = libuwac.la

AM_CPPFLAGS = \
	$(UWAC_CFLAGS) \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src 

libuwac_la_SOURCES = \
	src/uwac-os.h \
	src/uwac-os.c \
	src/uwac-priv.h \
	src/uwac-utils.c \
	src/uwac-utils.h \
	src/uwac-display.c \
	src/uwac-input.c \
	src/uwac-output.c \
	src/uwac-window.c
libuwac_la_LIBADD = $(UWAC_LIBS) 
	 

noinst_PROGRAMS = test_basic

test_basic_SOURCES = tests/test-basic.c
test_basic_LDADD = libuwac.la $(UWAC_LIBS)