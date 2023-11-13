#ifndef MLIST_ENTRY_H
#define MLIST_ENTRY_H

#include "stdint.h"
#include "cstring.h"

class MovListEntry
{
public:

    MovListEntry();

    bool readLineTxt(const CString &line);
    bool getMediaInfo(MovListEntry *entry);

    CString drive;
    CString directory;
    CString year;
    CString title;
    CString ftype;
    uint64_t fsize;
    uint64_t fmodified;
    CString mediainfo;

    CString titleKey;
    CString sortKey;

};

#endif // MLIST_ENTRY_H


