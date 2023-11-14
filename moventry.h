#ifndef MENTRY_H
#define MENTRY_H

#include "cstring.h"

typedef struct _MovEntry MovEntry;
MovEntry* mentry_new();
void mentry_free(MovEntry *entry);

bool mentry_readline(MovEntry *entry, const char *line);
bool mentry_get_media_info(MovEntry *entry, MovEntry *other);

struct _MovEntry
{
    CString *drive;
    CString *directory;
    CString *year;
    CString *title;
    CString *fext;
    uint64_t fsize;
    uint64_t ftime;

    CString *mediainfo;

    CString *titleKey;
    CString *sortKey;
};

#endif // MENTRY_H


