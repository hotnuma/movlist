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
    mlist_global.h \
    mlist_mediainfo.h

SOURCES = \
    0Temp.c \
    main.c \
    mlist.c \
    mlist_entry.c \
    mlist_global.c \
    mlist_mediainfo.cpp

DISTFILES = \
    install.sh \
    meson.build \


