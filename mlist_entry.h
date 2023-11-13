#ifndef MLIST_ENTRY_H
#define MLIST_ENTRY_H

#include "cstring.h"
#include <stdint.h>

typedef struct _MovListEntry MovListEntry;
MovListEntry* mlist_entry_new();
void mlist_entry_free(MovListEntry *entry);

bool mlist_entry_readline(MovListEntry *entry, const char *line);
bool mlist_entry_get_media_info(MovListEntry *entry, MovListEntry *other);

struct _MovListEntry
{
    CString *drive;
    CString *directory;
    CString *year;
    CString *title;
    CString *ftype;
    uint64_t fsize;
    uint64_t fmodified;

    CString *mediainfo;

    CString *titleKey;
    CString *sortKey;
};

#endif // MLIST_ENTRY_H


