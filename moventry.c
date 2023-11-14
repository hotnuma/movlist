#include "moventry.h"
#include <stdlib.h>
#include "cstringlist.h"
#include "global.h"

#define MLS_MINCOLS 7

MovEntry* mentry_new()
{
    MovEntry *entry = (MovEntry*) malloc(sizeof(MovEntry));

    entry->drive = cstr_new_size(16);
    entry->directory = cstr_new_size(64);
    entry->year = cstr_new_size(4);
    entry->title = cstr_new_size(24);
    entry->fext = cstr_new_size(12);
    entry->fsize = 0;
    entry->ftime = 0;
    entry->mediainfo = cstr_new_size(128);

    entry->titleKey = cstr_new_size(24);
    entry->sortKey = cstr_new_size(24);

    return entry;
}

void mentry_free(MovEntry *entry)
{
    if (!entry)
        return;

    cstr_free(entry->drive);
    cstr_free(entry->directory);
    cstr_free(entry->year);
    cstr_free(entry->title);
    cstr_free(entry->fext);
    cstr_free(entry->mediainfo);

    cstr_free(entry->titleKey);
    cstr_free(entry->sortKey);

    free(entry);
}

bool mentry_readline(MovEntry *entry, const char *line)
{
    CStringListAuto *allparts = cstrlist_new_size(16);
    cstrlist_split(allparts, line, SEP_TAB, true, true);
    int size = cstrlist_size(allparts);

    if (size < MLS_MINCOLS)
        return false;

    cstr_copy(entry->drive, cstrlist_at_str(allparts, 0));
    cstr_copy(entry->directory, cstrlist_at_str(allparts, 1));
    cstr_copy(entry->year, cstrlist_at_str(allparts, 2));
    cstr_copy(entry->title, cstrlist_at_str(allparts, 3));
    cstr_copy(entry->titleKey, c_str(entry->title));
    cstr_copy(entry->fext, cstrlist_at_str(allparts, 4));

    CStringAuto *temp = cstr_new_size(16);

    cstr_copy(temp, cstrlist_at_str(allparts, 5));
    entry->fsize = strtoull(c_str(temp), NULL, 10);

    cstr_copy(temp, cstrlist_at_str(allparts, 6));
    entry->ftime = strtoull(c_str(temp), NULL, 10);

    for (int i = MLS_MINCOLS; i < size; ++i)
    {
        if (i > MLS_MINCOLS)
            cstr_append(entry->mediainfo, SEP_TAB);

        cstr_append(entry->mediainfo, cstrlist_at_str(allparts, i));
    }

    cstr_copy(entry->sortKey, c_str(entry->titleKey));
    cstr_append(entry->sortKey, c_str(entry->year));

    return true;
}

bool mentry_get_media_info(MovEntry *entry, MovEntry *other)
{
    if (!other || cstr_isempty(other->mediainfo))
        return false;

    cstr_copy(entry->mediainfo, c_str(other->mediainfo));

    return true;
}


