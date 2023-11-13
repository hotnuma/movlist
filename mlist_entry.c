#include "mlist_entry.h"
#include <stdlib.h>
#include "cstringlist.h"
#include "mlist_global.h"

#define MLS_MINCOLS 7

MovListEntry* mlist_entry_new()
{
    MovListEntry *entry = (MovListEntry*) malloc(sizeof(MovListEntry));

    entry->drive = cstr_new_size(16);
    entry->directory = cstr_new_size(64);
    entry->year = cstr_new_size(4);
    entry->title = cstr_new_size(24);
    entry->ftype = cstr_new_size(12);

    entry->fsize = 0;
    entry->fmodified = 0;

    entry->mediainfo = cstr_new_size(128);

    entry->titleKey = cstr_new_size(24);
    entry->sortKey = cstr_new_size(24);

    return entry;
}

void mlist_entry_free(MovListEntry *entry)
{
    if (!entry)
        return;

    cstr_free(entry->drive);
    cstr_free(entry->directory);
    cstr_free(entry->year);
    cstr_free(entry->title);
    cstr_free(entry->ftype);

    cstr_free(entry->mediainfo);

    cstr_free(entry->titleKey);
    cstr_free(entry->sortKey);

    free(entry);
}

bool mlist_entry_readline(MovListEntry *entry, const char *line)
{
    CStringList *allParts = cstrlist_new_size(16);
    cstrlist_split(allParts, line, SEP_TAB, true, true);

    int size = cstrlist_size(allParts);

    if (size < MLS_MINCOLS)
        return false;

    cstr_copy(entry->drive, cstrlist_at_str(allParts, 0));
    cstr_copy(entry->directory, cstrlist_at_str(allParts, 1));
    cstr_copy(entry->year, cstrlist_at_str(allParts, 2));
    cstr_copy(entry->title, cstrlist_at_str(allParts, 3));
    cstr_copy(entry->titleKey, c_str(entry->title));
    cstr_copy(entry->ftype, cstrlist_at_str(allParts, 4));

    CStringAuto *temp = cstr_new_size(16);

    cstr_copy(temp, cstrlist_at_str(allParts, 5));
    entry->fsize = strtoull(c_str(temp), NULL, 10);

    cstr_copy(temp, cstrlist_at_str(allParts, 6));
    entry->fmodified = strtoull(c_str(temp), NULL, 10);

    for (int i = MLS_MINCOLS; i < size; ++i)
    {
        if (i > MLS_MINCOLS)
            cstr_append(entry->mediainfo, SEP_TAB);

        cstr_append(entry->mediainfo, cstrlist_at_str(allParts, i));
    }

    cstr_copy(entry->sortKey, c_str(entry->titleKey));
    cstr_append(entry->sortKey, c_str(entry->year));

    return true;
}

bool mlist_entry_get_media_info(MovListEntry *entry, MovListEntry *other)
{
    if (!other || cstr_isempty(other->mediainfo))
        return false;

    cstr_copy(entry->mediainfo, c_str(other->mediainfo));

    return true;
}


