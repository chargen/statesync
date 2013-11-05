# Configuration file for statesync

AC_INIT([statesync], [0.1] [], [statesync], [])

AM_INIT_AUTOMAKE

# We are going to use silent builds which have a much 
# nicer output. More information can be foudn here: 
#     https://live.gnome.org/GnomeGoals/NicerBuilds 
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])]) 

AC_PROG_CC
AM_PROG_CC_C_O

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_SRCDIR([src/main.c]) 

# Check for libaries using pkg-config
PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.34])

# Use C99 compilation mode
CFLAGS="$CFLAGS -std=c99"

AC_CONFIG_FILES([
   Makefile
   src/Makefile
])
AC_OUTPUT

