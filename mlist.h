#ifndef MLIST_H
#define MLIST_H

#include "CStringList.h"
#include "CFile.h"
#include "CList.h"
#include "MovListEntry.h"

class MovList
{
public:

    MovList();

    int     size() {return _entryList.size();}
    void    sortByKey();
    MovListEntry* find(MovListEntry *entry);

    bool    execute(int argc, char **argv);

    bool    readParams(const CString &inipath, const CString &section);

    bool    readFile(const CString &filepath, CString fname = "");
    bool    writeFile(const CString &filepath);

private:

    bool    _readDirectory(MovList &movlist,
                           const CString &basedir,
                           const CString &subdir,
                           const CString &drivename);

    CString _getDefaultHeader();
    bool    _writeHeader(CString &buffer);

    CStringList _inlist;
    CString     _outpath;

    CString     _optinclude;

    bool        _optgroup = false;
    int         _optminsize = 50;
    bool        _optinfos = false;
    bool        _optxls = false;

    CList       _entryList;

};

#endif // MLIST_H


