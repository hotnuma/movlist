#ifndef MOVLIST_H
#define MOVLIST_H

#include "cstringlist.h"
#include "cfile.h"
#include "clist.h"
#include "moventry.h"

typedef struct _MovList MovList;
MovList* mlist_new();
void    mlist_free(MovList *list);

int     mlist_size(MovList *list);
void    mlist_sort(MovList *list);
MovEntry* mlist_find(MovList *list, MovEntry *entry);

bool    mlist_execute(MovList *list, int argc, char **argv);

bool    mlist_readParams(MovList *list, const char *inipath, const char *section);
bool    mlist_readFile(MovList *list, const char *filepath, const char *drive);
bool    mlist_writeFile(MovList *list, const char *filepath);

struct _MovList
{
    CStringList *drivelist;
    CString     *outpath;

    CString     *opt_include;
    int         opt_minsize;
    bool        opt_media;
    bool        opt_titlesort;
    bool        opt_ods;

    CList       *entryList;

};

#endif // MOVLIST_H


