TEMPLATE = app
TARGET = newlist
CONFIG = c99 c++11 link_pkgconfig
DEFINES = UNICODE _GNU_SOURCE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc
PKGCONFIG += libmediainfo

HEADERS = \
    mlist.h \
    mlist_entry.h \
    MovMediaInfo.h \
    mlist_global.h

SOURCES = \
    0Temp.c \
    main.c \
    mlist.c \
    mlist_entry.c \
    MovMediaInfo.cpp \
    mlist_global.c

DISTFILES = \
    install.sh \
    meson.build \


