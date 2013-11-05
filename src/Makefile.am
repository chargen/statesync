bin_PROGRAMS = statesync

statesync_SOURCES = main.c \
                    add.h add.c \
					FileEntry.h FileEntry.c \
					util.h util.c \
					receive.h receive.c \
					send.h send.c \
					status.h status.c \
					types.h \
				    statesync.h

statesync_CFLAGS = $(AM_CFLAGS) \
                   $(GLIB_CFLAGS)

statesync_LDADD = $(AM_LDADD) \
                  $(GLIB_LIBS) \
                  -lcrypto

MAINTAINERCLEANFILES = Makefile.in
