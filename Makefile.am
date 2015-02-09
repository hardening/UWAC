lib_LTLIBRARIES = libuwac.la

BUILT_SOURCES =

AM_CPPFLAGS = \
	$(UWAC_CFLAGS) \
	-I$(top_builddir)/src \
	-I$(top_builddir)/protocol \
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
	 
nodist_libuwac_la_SOURCES =					\
	protocol/xdg-shell-protocol.c		\
	protocol/xdg-shell-client-protocol.h

BUILT_SOURCES += $(nodist_libuwac_la_SOURCES)



noinst_PROGRAMS = test_basic test_window

test_basic_SOURCES = tests/test-basic.c
test_basic_LDADD = libuwac.la $(UWAC_LIBS)

test_window_SOURCES = tests/test-window.c
test_window_LDADD = libuwac.la $(UWAC_LIBS)



protocol/%-protocol.c : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) code < $< > $@

protocol/%-server-protocol.h : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) server-header < $< > $@

protocol/%-client-protocol.h : $(top_srcdir)/protocol/%.xml
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(wayland_scanner) client-header < $< > $@
