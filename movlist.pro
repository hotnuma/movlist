TEMPLATE = app
TARGET = newlist
CONFIG = c99 c++11 link_pkgconfig
DEFINES = UNICODE _GNU_SOURCE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc
PKGCONFIG += libmediainfo

HEADERS = \
    global.h \
    mlist.h \
    mlist_entry.h \
    MovMediaInfo.h \

SOURCES = \
    0Temp.c \
    global.c \
    main.c \
    mlist.c \
    mlist_entry.c \
    MovMediaInfo.cpp \

DISTFILES = \
    install.sh \
    meson.build \


