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
    mediainfo.h \
    moventry.h \
    movlist.h

SOURCES = \
    0Temp.c \
    global.c \
    main.c \
    mediainfo.cpp \
    moventry.c \
    movlist.c

DISTFILES = \
    install.sh \
    meson.build \


