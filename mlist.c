#include "mlist.h"

#include "mlist_global.h"
#include "mlist_mediainfo.h"

#include <cfileinfo.h>
#include <cdirparser.h>
#include <cinifile.h>
//#include <cfile.h>
#include <libpath.h>
#include <libstr.h>
#include <time.h>
#include <string.h>

#include <print.h>

#define DEFAULT_SECTION "Default"
#define DEFAULT_HEADER "Default"

int _compare(const void *entry1, const void *entry2)
{
    const MovListEntry *e1 = *((MovListEntry**) entry1);
    const MovListEntry *e2 = *((MovListEntry**) entry2);

    return cstr_compare(e1->sortKey, c_str(e2->sortKey), true);
}

MovList* mlist_new()
{
    MovList *list = (MovList*) malloc(sizeof(MovList));

    list->inlist = cstrlist_new_size(16);
    list->outpath = cstr_new_size(128);
    list->optinclude = cstr_new_size(128);

    list->optgroup = false;
    list->optminsize = 50;
    list->optinfos = false;
    list->optxls = false;

    list->entryList = clist_new(2048, (CDeleteFunc) mlist_entry_free);

    return list;
}

void pritem_free(MovList *list)
{
    if (!list)
        return;

    cstrlist_free(list->inlist);
    cstr_free(list->outpath);
    cstr_free(list->optinclude);
    clist_free(list->entryList);

    free(list);
}

bool _mlist_read_directory(MovList *list,
                           MovList *movlist,
                           CString *basedir,
                           CString *subdir,
                           CString *drivename);

CString _getDefaultHeader();
bool _writeHeader(CString *buffer);

int mlist_size(MovList *list)
{
    return clist_size(list->entryList);
}

void mlist_sortByKey(MovList *list)
{
    clist_sort(list->entryList, _compare);
}

MovListEntry* mlist_find(MovList *list, MovListEntry *entry)
{
    if (!entry)
        return NULL;

    int size = clist_size(list->entryList);

    for (int i = 0; i < size; ++i)
    {
        MovListEntry *current = (MovListEntry*) clist_at(list->entryList, i);

        if (cstr_compare(current->title, c_str(entry->title), true) == 0
            && current->fsize == entry->fsize
            && current->fmodified == entry->fmodified)
            return current;
    }

    return NULL;
}

bool mlist_execute(MovList *list, int argc, char **argv)
{
    int n = 1;

    CStringAuto *fname = cstr_new_size(16);
    CStringAuto *section = cstr_new_size(16);
    cstr_copy(section, "Default");

    while (n < argc)
    {
        const char *part = argv[n];

        if (strcmp(part, "-se") == 0)
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            cstr_copy(section, argv[n]);
        }

        else if (strcmp(part, "-mi") == 0)
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            list->optminsize = atoi(argv[n]);
        }

        else if (strcmp(part, "-me") == 0)
        {
            list->optinfos = true;
        }

        else if (strcmp(part, "-xl") == 0)
        {
            list->optxls = true;
        }

        else if (strcmp(part, "-i") == 0)
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            // get input ini files such as "Films.ini".

            part = argv[n];

            if (strlen(part) < 1
                || str_endswith(part, ".ini", true) == false
                || strchr(part, '/') != NULL)
            {
                print("*** Invalid file: %s", part);

                return false;
            }

            // put unique base names into a list.

            cstr_copy(fname, part);
            path_strip_ext(fname, true);

            if (cstrlist_find(list->inlist, c_str(fname), true) == -1)
                cstrlist_append(list->inlist, c_str(fname));
        }

        else if (strcmp(part, "-o") == 0)
        {
            if (++n >= argc)
            {
                print("*** Missing parameter.");
                return false;
            }

            cstr_copy(list->outpath, argv[n]);

            if (!cstr_endswith(list->outpath, ".txt", true))
            {
                print("*** Invalid file path: %s", c_str(list->outpath));
                return false;
            }
        }

        else
        {
            print("Invalid option : %s", argv[n]);
            return false;
        }

        n++;
    }

    if (cstrlist_isempty(list->inlist) || cstr_isempty(list->outpath))
    {
        print("*** Missing option.");
        return false;
    }

    // parse inifile base names.

    CStringAuto *fullpath = cstr_new_size(128);

    CStringAuto *basedir = cstr_new_size(128);
    CStringListAuto *inclist =  cstrlist_new_size(24);
    CStringAuto *fulldir = cstr_new_size(128);

    int size = cstrlist_size(list->inlist);

    for (int i = 0; i < size; ++i)
    {
        CString *drivename = cstrlist_at(list->inlist, i);

        print(LINE1);
        print(" name  : %s", c_str(drivename));

        cstr_clear(fullpath);

        // no plugged drive, read input file.

        if (!get_fullpath(fullpath, c_str(drivename)))
        {
            // External drive not plugged.
            if (file_exists(c_str(list->outpath))
                && !mlist_readFile(list, list->outpath, drivename))
                return false;

            print("");
        }

        // parse plugged drive.

        else
        {
            print(" drive : %s", c_str(fullpath));

            // read ini file
            if (!mlist_readParams(list, c_str(fullpath), c_str(section)))
            {
                print("*** Error reading file: %s", c_str(fullpath));
                return false;
            }

            MovList *movlist = mlist_new();

            if (list->optinfos && file_exists(c_str(list->outpath)))
            {
                if (!mlist_readFile(movlist, list->outpath, drivename))
                {
                    mlist_free(movlist);
                    return false;
                }
            }

            print(LINE2);
            print("");

            // parse included sub directories.

            path_dirname(basedir, c_str(fullpath));
            cstrlist_split(inclist, c_str(list->optinclude), ",", false, true);

            int sz = cstrlist_size(inclist);

            for (int i = 0; i < sz; ++i)
            {
                CString *subdir = cstrlist_at(inclist, i);

                path_join(fulldir, c_str(basedir), c_str(subdir));
                print(" + %s", c_str(fulldir));

                if (!_mlist_read_directory(list,
                                           movlist,
                                           basedir,
                                           subdir,
                                           drivename))
                {
                    mlist_free(movlist);
                    return false;
                }

                print("");
            }

            mlist_free(movlist);
        }
    }

    mlist_sortByKey(list);

    if (!mlist_writeFile(list, list->outpath))
        return false;

    if (!list->optxls)
        return true;

    CStringAuto *reportpath = cstr_new_copy(list->outpath);
    path_strip_ext(reportpath, true);
    cstr_append(reportpath, REPORT_EXT);

    if (!mlist_writeFile(list, reportpath))
        return false;

    return true;
}

bool mlist_readParams(MovList *list, const char *inipath, const char *section)
{
    if (!file_exists(inipath))
        return false;

    CIniFileAuto *inifile = cinifile_new();
    cinifile_read(inifile, inipath);

    CIniSection *iniSection = cinifile_section(inifile, section);

    if (!iniSection)
        return false;

    cinisection_value(iniSection, list->optinclude, "include", "");

    if (cstr_isempty(list->optinclude))
        return false;

    return true;
}

bool _mlist_read_directory(MovList *list, MovList *movlist,
                           CString *basedir, CString *subdir, CString *drivename)
{
    CStringAuto *directory = cstr_new_size(128);
    path_join(directory, c_str(basedir), c_str(subdir));

    if (cstr_isempty(directory) || !dir_exists(c_str(directory)))
    {
        print("*** Can't read directory %s", c_str(directory));
        return false;
    }

    CDirParserAuto *dir = cdirparser_new();
    if (!cdirparser_open(dir, c_str(directory), CDP_SUBDIRS | CDP_FILES))
        return false;

    int count = 0;

    CStringAuto *filepath = cstr_new_size(128);
    CFileInfoAuto *fileinfo = cfileinfo_new();

    while (cdirparser_read(dir, filepath, NULL))
    {
        if (!cstr_endswith(filepath, ".avi", false)
            && !cstr_endswith(filepath, ".divx", false)
            && !cstr_endswith(filepath, ".flv", false)
            && !cstr_endswith(filepath, ".m2ts", false)
            && !cstr_endswith(filepath, ".mkv", false)
            && !cstr_endswith(filepath, ".mp4", false)
            && !cstr_endswith(filepath, ".mpg", false)
            && !cstr_endswith(filepath, ".ts", false)
            && !cstr_endswith(filepath, ".vob", false))
        {
            continue;
        }

        cfileinfo_read(fileinfo, c_str(filepath));

        // skip files that are to small.
        uint64_t fsize = cfileinfo_size(fileinfo);
        if (fsize < (uint64_t) (list->optminsize * 1000000))
            continue;

        if (count < 1)
            print("");

        print("   %s", c_str(filepath));
        ++count;

        MovListEntry *entry = mlist_entry_new();
        // ******* mem leak

        cstr_copy(entry->drive, c_str(drivename));
        cstr_copy(entry->directory, c_str(subdir));
        entry->fsize = fsize;
        entry->fmodified = cfileinfo_mtime(fileinfo);

        const char *p = path_ext(c_str(filepath), true);
        if (p)
        {
            ++p;
            cstr_copy(entry->ftype, p);
        }

        p = path_basename_ptr(c_str(filepath));

        if (!p)
            p = c_str(filepath);

        cstr_copy(entry->title, p);
        path_strip_ext(entry->title, true);

        p = c_str(entry->title);
        int pos = strstr(p, " - ") - p;

        if (pos == 4 && p && strlen(p) > 7)
        {
            entry->year = temp.left(pos);
            entry->title = temp.mid(pos + 3);
            //print("year : %s", entry->year.c_str());
            //print("title : %s", entry->title.c_str());
        }

        //entry->titleKey = getTitleKey(entry->title);
        entry->titleKey = entry->title;
        entry->sortKey = entry->titleKey;
        entry->sortKey += entry->year;

        if (_optinfos)
        {
            MovListEntry *found = movlist.find(entry);
            if (found)
            {
                entry->getMediaInfo(found);
            }

            if (entry->mediainfo.isEmpty())
            {
                print("    > read media infos...\n");

                if (!getMediaInfo(filepath, entry->mediainfo))
                {
                    print("*** error: read media error");
                }

                //print(entry->mediainfo);
            }
        }

        _entryList.append(entry);
    }

    return true;
}

bool mlist_readFile(MovList *list, CString *filepath, CString *fname)
{
    print(" read  : %s", filepath.c_str());

    // Parse all lines.
    int count = 0;

    if (!fname.isEmpty())
        fname += SEP_TAB;

    CFile file;

    if (!file.read(filepath))
    {
        print("*** Can't open input file: %s", filepath.c_str());
        return false;
    }

    CString line;

    while (file.getLine(line))
    {
        if (count == 0)
        {
            CString header = _getDefaultHeader();

            if (!line.startsWith(header))
            {
                print("*** Invalid header.");

                return false;
            }

            ++count;
            continue;
        }

        if (line.isEmpty())
            continue;

        if (!fname.isEmpty() && !line.startsWith(fname))
            continue;

        MovListEntry *entry = new MovListEntry();

        if (!entry->readLineTxt(line))
        {
            print("*** Invalid line in %s", filepath.c_str());

            delete entry;
            return false;
        }

        _entryList.append(entry);

        ++count;
    }

    print("         %i lines", count);

    return true;
}

bool mlist_writeFile(MovList *list, CString *filepath)
{
    CString buffer;

    bool report = false;
    if (filepath.endsWith(REPORT_EXT))
        report = true;

    _writeHeader(buffer);

    //foreach (MovListEntry *entry, _entryList)

    int size = _entryList.size();

    for (int i = 0; i < size; ++i)
    {
        MovListEntry *entry = (MovListEntry*) _entryList[i];

        buffer += entry->drive;
        buffer += SEP_TAB;
        buffer += entry->directory;
        buffer += SEP_TAB;
        buffer += entry->year;
        buffer += SEP_TAB;
        buffer += entry->title;
        buffer += SEP_TAB;
        buffer += entry->ftype;

        if (report)
        {
            buffer += SEP_TAB;
            buffer += uint64ToStr(entry->fsize / 1000000);

            buffer += SEP_TAB;

            time_t ltime = entry->fmodified / 1000;
            struct tm *mytime = localtime(&ltime);

            buffer += strFmt("%i/%.2i/%.2i-%.2i:%.2i",
                             mytime->tm_year + 1900, mytime->tm_mon + 1, mytime->tm_mday,
                             mytime->tm_hour, mytime->tm_min);
        }
        else
        {
            buffer += SEP_TAB;
            buffer += uint64ToStr(entry->fsize);
            buffer += SEP_TAB;
            buffer += uint64ToStr(entry->fmodified);
        }

        if (_optinfos)
        {
            buffer += SEP_TAB;
            buffer += entry->mediainfo;
        }

        buffer += "\r\n";
    }

    if (report)
    {
        //wchar_t *wstr = utf8ToWchar(buffer);
        //int size = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
        //char* buff = (char*) malloc(size * sizeof(char));
        //WideCharToMultiByte(CP_ACP, 0, wstr, -1, buff, size, NULL, NULL);
        //free(wstr);

        CFile::write(filepath, buffer);

        //free(buff);
    }
    else
    {
        CFile::write(filepath, buffer);
    }

    return true;
}

CString  MovList::_getDefaultHeader()
{
    CString sep = SEP_TAB;

    CString result = "Drive";
    result += sep;
    result += "Directory";
    result += sep;
    result += "Year";
    result += sep;
    result += "Title";
    result += sep;
    result += "Type";
    result += sep;
    result += "Size";
    result += sep;
    result += "Modified";

    return result;
}

bool MovList::_writeHeader(CString &buffer)
{
    CString sep = SEP_TAB;

    CString line = _getDefaultHeader();

    if (_optinfos)
    {
        line += sep;
        getMediaHeader(line);
    }

    line += "\r\n";

    buffer += line;

    return true;
}


