AC_PREREQ([2.64])

m4_define([uwac_major_version], [1])
m4_define([uwac_minor_version], [0])
m4_define([uwac_micro_version], [0])
m4_define([uwac_version],
          [uwac_major_version.uwac_minor_version.uwac_micro_version])

AC_INIT([libuwac],
        [uwac_version],
        [http://github.com/hardening/libuwac],
        [libuwac],
        [http://wayland.freedesktop.org/])

AC_SUBST([UWAC_VERSION_MAJOR], [uwac_major_version])
AC_SUBST([UWAC_VERSION_MINOR], [uwac_minor_version])
AC_SUBST([UWAC_VERSION_MICRO], [uwac_micro_version])
AC_SUBST([UWAC_VERSION], [uwac_version])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz subdir-objects])

AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT


AC_CHECK_FUNCS([mkostemp strchrnul initgroups posix_fallocate])

UWAC_MODULES="wayland-client >= 1.5.91 pixman-1"

AC_ARG_ENABLE(xkbcommon,
	      AS_HELP_STRING([--disable-xkbcommon], [Disable libxkbcommon
			      support: This is only useful in environments
			      where you do not have a hardware keyboard. If
			      libxkbcommon support is disabled clients will not
			      be sent a keymap and and must know how to
			      interpret the keycode sent for any key event.]),,
	      enable_xkbcommon=yes)
if test x$enable_xkbcommon = xyes; then
	AC_DEFINE(ENABLE_XKBCOMMON, [1], [Build UWAC with libxkbcommon support])
	UWAC_MODULES="$UWAC_MODULES xkbcommon >= 0.3.0"
fi

PKG_CHECK_MODULES(UWAC, [$UWAC_MODULES])

AC_PATH_PROG([wayland_scanner], [wayland-scanner])
if test x$wayland_scanner = x; then
	AC_MSG_ERROR([wayland-scanner is needed to compile weston])
fi

PKG_CHECK_MODULES(WAYLAND_SCANNER, wayland-scanner)

AC_CONFIG_FILES([
Makefile
uwac.pc
])
AC_OUTPUT
