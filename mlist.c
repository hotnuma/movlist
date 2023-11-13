#include "MovList.h"
#include "Global.h"
#include "MovMediaInfo.h"

#include <CFileInfo.h>
#include <CDirParser.h>
#include <CIniFile.h>
#include <libfile.h>
#include <libpath.h>

#include <time.h>
#include <string.h>

#include <print.h>

#define DEFAULT_SECTION "Default"
#define DEFAULT_HEADER "Default"

int _compare(const void *entry1, const void *entry2)
{
    const MovListEntry *e1 = *((MovListEntry**) entry1);
    const MovListEntry *e2 = *((MovListEntry**) entry2);

    return e1->sortKey.compare(e2->sortKey);
}

DELETEFUNC(MovListEntry)

MovList::MovList()
{
    SETDELETEFUNC(&_entryList, MovListEntry);
}

void MovList::sortByKey()
{
    _entryList.sort(_compare);
}

MovListEntry* MovList::find(MovListEntry *entry)
{
    if (!entry)
        return nullptr;

    int size = _entryList.size();

    for (int i = 0; i < size; ++i)
    {
        MovListEntry *current = (MovListEntry*) _entryList[i];

        if (current->title == entry->title
            && current->fsize == entry->fsize
            && current->fmodified == entry->fmodified)
            return current;
    }

    return nullptr;
}

bool MovList::execute(int argc, char **argv)
{
    int n = 1;

    CString section = "Default";

    while (n < argc)
    {
        CString part = argv[n];

        if (part == "-se")
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            section = argv[n];
        }

        else if (part == "-mi")
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            CString temp = argv[n];
            _optminsize = temp.toInt();
        }

        else if (part == "-me")
        {
            _optinfos = true;
        }

        else if (part == "-xl")
        {
            _optxls = true;
        }

        else if (part == "-i")
        {
            if (++n >= argc)
            {
                print("Missing parameter.");
                return false;
            }

            // get input ini files such as "Films.ini".

            CString fname = argv[n];

            if (fname.isEmpty()
                || !fname.endsWith(".ini")
                || fname.contains("/"))
            {
                print("*** Invalid file: %s", fname.c_str());

                return false;
            }

            // put unique base names into a list.

            fname = pathBaseName(fname);

            if (_inlist.find(fname) == -1)
                _inlist.append(fname);
        }

        else if (part == "-o")
        {
            if (++n >= argc)
            {
                print("*** Missing parameter.");
                return false;
            }

            _outpath = argv[n];

            if (!_outpath.endsWith(".txt"))
            {
                print("*** Invalid file path: %s", _outpath.c_str());
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

    if (_inlist.isEmpty() || _outpath == "")
    {
        print("*** Missing option.");
        return false;
    }

    // parse inifile base names.

    int size = _inlist.size();

    for (int i = 0; i < size; ++i)
    {
        CString &drivename = _inlist[i];

        print(LINE1);
        print(" name  : %s", drivename.c_str());

        CString fullpath;

        // no plugged drive, read input file.
        if (!getFullPath(drivename, fullpath))
        {
            // External drive not plugged.
            if (fileExists(_outpath)
                && !readFile(_outpath, drivename))
                return false;

            //print(LINE2);
            print("");
        }

        // parse plugged drive.
        else
        {
            print(" drive : %s", fullpath.c_str());

            // read ini file
            if (!readParams(fullpath, section))
            {
                print("*** Error reading file: %s", fullpath.c_str());
                return false;
            }

            MovList movlist;

            if (_optinfos && fileExists(_outpath))
            {
                if (!movlist.readFile(_outpath, drivename))
                {
                    return false;
                }
            }

            print(LINE2);
            print("");

            // parse included sub directories.

            CString basedir = pathDirName(fullpath);
            CStringList inclist = _optinclude.split(",");

            int sz = inclist.size();

            for (int i = 0; i < sz; ++i)
            {
                CString subdir = inclist[i];
                CString fulldir = pathJoin(basedir, subdir);

                print(" + %s", fulldir.c_str());

                if (!_readDirectory(movlist,
                                    basedir,
                                    subdir,
                                    drivename))
                {
                    return false;
                }

                print("");
            }
        }
    }

    sortByKey();

    if (!writeFile(_outpath))
        return false;

    if (!_optxls)
        return true;

    CString reportpath = pathBaseName(_outpath);

    reportpath += REPORT_EXT; // ".xls";

    if (!writeFile(reportpath))
        return false;

    return true;
}

bool MovList::readParams(const CString &inipath, const CString &section)
{
    if (!fileExists(inipath))
        return false;

    CIniFile inifile;
    inifile.open(inipath);

    CIniSection *iniSection = inifile.section(section);

    if (!iniSection)
        return false;

    _optinclude = iniSection->value("include");
    if (_optinclude == "")
        return false;

    return true;
}

bool MovList::_readDirectory(MovList &movlist,
                             const CString &basedir,
                             const CString &subdir,
                             const CString &drivename)
{
    CString directory = pathJoin(basedir, subdir);

    if (directory.isEmpty() || !dirExists(directory))
    {
        print("*** Can't read directory %s", directory.c_str());
        return false;
    }

    CDirParser dir;
    if (!dir.open(directory, CDP_SUBDIRS | CDP_FILES))
        return false;

    int count = 0;
    CString filepath;

    while (dir.read(filepath))
    {
        if (!filepath.endsWith(".avi", false)
            && !filepath.endsWith(".divx", false)
            && !filepath.endsWith(".flv", false)
            && !filepath.endsWith(".m2ts", false)
            && !filepath.endsWith(".mkv", false)
            && !filepath.endsWith(".mp4", false)
            && !filepath.endsWith(".mpg", false)
            && !filepath.endsWith(".ts", false)
            && !filepath.endsWith(".vob", false))
            continue;

        CFileInfo fileinfo(filepath);

        // skip files that are to small.
        uint64_t fsize = fileinfo.size();
        if (fsize < (uint64_t) (_optminsize * 1000000))
            continue;

        if (count < 1)
            print("");

        print("   %s", filepath.c_str());
        ++count;

        MovListEntry *entry = new MovListEntry();

        entry->drive = drivename;
        entry->directory = subdir;
        entry->fsize = fsize;
        entry->fmodified = fileinfo.mtime();

        CString ext = pathExt(filepath);
        if (ext.startsWith("."))
            entry->ftype = ext.mid(1);
        else
            entry->ftype = ext;

        CString temp = pathFileName(filepath);
        entry->title = pathBaseName(temp);

        temp = entry->title;
        int pos = strstr(temp, " - ") - temp.c_str();

        if (pos == 4 && temp.size() > 7)
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

bool MovList::readFile(const CString &filepath, CString fname)
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

bool MovList::writeFile(const CString &filepath)
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


