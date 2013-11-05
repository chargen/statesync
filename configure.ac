# Configuration file for statesync

AC_INIT([statesync], [0.1] [], [statesync], [])

AM_INIT_AUTOMAKE


# We are going to use silent builds which have a much 
# nicer output. More information can be foudn here: 
#     https://live.gnome.org/GnomeGoals/NicerBuilds 
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])]) 

AC_ARG_ENABLE([clang],
                [AS_HELP_STRING([--enable-clang],[use clang instead of gcc as C compiler.])])

if test "x$enable_clang" = "xyes"; then
    # clang uses c99 mode by default, so we just set CC to clang and we are done
    CC="clang";
else
    # We do not need to set CC as the default is gcc, but we need to set it to
    # use C99 compilation mode
    CFLAGS="$CFLAGS -std=c99";
fi

AC_PROG_CC
AM_PROG_CC_C_O

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_SRCDIR([src/main.c]) 

# Check for libaries using pkg-config
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.34])



AC_CONFIG_FILES([
   Makefile
   src/Makefile
])
AC_OUTPUT

