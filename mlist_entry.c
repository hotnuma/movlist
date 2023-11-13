#include "mlist_entry.h"
#include <stdlib.h>
#include "cstringlist.h"
#include "mlist_global.h"

#define MLS_MINCOLS 7

MovListEntry::MovListEntry()
{
}

bool MovListEntry::readLineTxt(const CString &line)
{
    CStringList allParts = line.split(SEP_TAB);

    int size = allParts.size();

    if (size < MLS_MINCOLS)
        return false;

    drive = allParts[0];
    directory = allParts[1];
    year = allParts[2];
    title = allParts[3];
    //titleKey = getTitleKey(title);
    titleKey = title;
    ftype = allParts[4];
    CString temp = allParts[5];
    fsize = strtoull(temp, nullptr, 10); // temp.toLongLong();
    temp = allParts[6];
    fmodified = strtoull(temp, nullptr, 10); //.toLongLong();

    for (int i = MLS_MINCOLS; i < size; i++)
    {
        if (i > MLS_MINCOLS)
            mediainfo += SEP_TAB;

        mediainfo += allParts[i];
    }

    sortKey = strFmt("%s%s", titleKey.c_str(), year.c_str());

    return true;
}

bool MovListEntry::getMediaInfo(MovListEntry *entry)
{
    if (!entry || entry->mediainfo == "")
        return false;

    mediainfo = entry->mediainfo;

    return true;
}


