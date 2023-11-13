#ifndef MLIST_H
#define MLIST_H

#include "cstringlist.h"
#include "cfile.h"
#include "clist.h"
#include "mlist_entry.h"

typedef struct _MovList MovList;
MovList* mlist_new();
void    mlist_free(MovList *list);

int     mlist_size(MovList *list);
void    mlist_sortByKey(MovList *list);
MovListEntry* mlist_find(MovList *list, MovListEntry *entry);

bool    mlist_execute(MovList *list, int argc, char **argv);

bool    mlist_readParams(MovList *list, const char *inipath, const char *section);
bool    mlist_readFile(MovList *list, const char *filepath, const char *drivename);
bool    mlist_writeFile(MovList *list, const char *filepath);

struct _MovList
{
    CStringList *inlist;
    CString     *outpath;

    CString     *optinclude;

    bool        optgroup;
    int         optminsize;
    bool        optinfos;
    bool        optxls;

    CList       *entryList;

};

#endif // MLIST_H


