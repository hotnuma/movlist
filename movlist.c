#include "movlist.h"

#include "global.h"
#include "mediainfo.h"

#include <cfileinfo.h>
#include <cdirparser.h>
#include <cinifile.h>
#include <libpath.h>
#include <libstr.h>
#include <time.h>

#include <print.h>

#define DEFAULT_SECTION "Default"
#define DEFAULT_HEADER "Drive\tDirectory\tYear\tTitle\tType\tSize\tModified"
#define MEDIA_HEADER "Application\tBitRate\tDuration\tVideo\tWidth\tHeigth\tAspect\tFrameRate\tAudio"

int _compare(const void *entry1, const void *entry2)
{
    const MovEntry *e1 = *((MovEntry**) entry1);
    const MovEntry *e2 = *((MovEntry**) entry2);

    return cstr_compare(e1->sortKey, c_str(e2->sortKey), true);
}

MovList* mlist_new()
{
    MovList *list = (MovList*) malloc(sizeof(MovList));

    list->drivelist = cstrlist_new_size(16);
    list->outpath = cstr_new_size(128);

    list->opt_include = cstr_new_size(128);
    list->opt_minsize = 0; //50;
    list->opt_media = false;
    list->opt_ods = false;

    list->entryList = clist_new(2048, (CDeleteFunc) mentry_free);

    return list;
}

void mlist_free(MovList *list)
{
    if (!list)
        return;

    cstrlist_free(list->drivelist);
    cstr_free(list->outpath);
    cstr_free(list->opt_include);
    clist_free(list->entryList);

    free(list);
}

bool _mlist_read_directory(MovList *list, MovList *movlist,
                           CString *basedir, CString *subdir,
                           const char *drivename);

void _mlist_writeHeader(MovList *list, CFile *file);

int mlist_size(MovList *list)
{
    return clist_size(list->entryList);
}

void mlist_sortByKey(MovList *list)
{
    clist_sort(list->entryList, _compare);
}

MovEntry* mlist_find(MovList *list, MovEntry *entry)
{
    if (!entry)
        return NULL;

    int size = clist_size(list->entryList);

    for (int i = 0; i < size; ++i)
    {
        MovEntry *current = (MovEntry*) clist_at(list->entryList, i);

        if (cstr_compare(current->title, c_str(entry->title), true) == 0
            && current->fsize == entry->fsize
            && current->ftime == entry->ftime)
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

            list->opt_minsize = atoi(argv[n]);
        }

        else if (strcmp(part, "-me") == 0)
        {
            list->opt_media = true;
        }

        else if (strcmp(part, "-xl") == 0)
        {
            list->opt_ods = true;
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

            if (cstrlist_find(list->drivelist, c_str(fname), true) == -1)
                cstrlist_append(list->drivelist, c_str(fname));
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

    if (cstrlist_isempty(list->drivelist) || cstr_isempty(list->outpath))
    {
        print("*** Missing option.");
        return false;
    }

    // parse inifile base names.

    CStringAuto *inipath = cstr_new_size(128);
    CStringAuto *rootdir = cstr_new_size(128);

    CStringListAuto *sublist =  cstrlist_new_size(24);
    CStringAuto *fulldir = cstr_new_size(128);

    int size = cstrlist_size(list->drivelist);

    for (int i = 0; i < size; ++i)
    {
        const char *drive = c_str(cstrlist_at(list->drivelist, i));

        print(LINE1);
        print(" name  : %s", drive);

        cstr_clear(inipath);

        // search plugged drive from ini file name

        if (!get_fullpath(inipath, drive))
        {
            // drive not plugged, read txt file

            if (file_exists(c_str(list->outpath))
                && !mlist_readFile(list, c_str(list->outpath), drive))
                return false;

            print("");
        }

        // parse plugged drive.

        else
        {
            print(" drive : %s", c_str(inipath));

            // read ini file

            if (!mlist_readParams(list, c_str(inipath), c_str(section)))
            {
                print("*** Error reading file: %s", c_str(inipath));
                return false;
            }

            MovList *movlist = mlist_new();

            if (list->opt_media && file_exists(c_str(list->outpath)))
            {
                if (!mlist_readFile(movlist, c_str(list->outpath), drive))
                {
                    mlist_free(movlist);
                    return false;
                }
            }

            print(LINE2);
            print("");

            // parse included sub directories

            path_dirname(rootdir, c_str(inipath));
            cstrlist_split(sublist, c_str(list->opt_include), ",", false, true);

            int sz = cstrlist_size(sublist);

            for (int i = 0; i < sz; ++i)
            {
                CString *subdir = cstrlist_at(sublist, i);

                path_join(fulldir, c_str(rootdir), c_str(subdir));
                print(" + %s", c_str(fulldir));

                if (!_mlist_read_directory(list, movlist, rootdir, subdir, drive))
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

    if (!mlist_writeFile(list, c_str(list->outpath)))
        return false;

    if (!list->opt_ods)
        return true;

    CStringAuto *reportpath = cstr_new_copy(list->outpath);
    path_strip_ext(reportpath, true);
    cstr_append(reportpath, REPORT_EXT);

    if (!mlist_writeFile(list, c_str(reportpath)))
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

    cinisection_value(iniSection, list->opt_include, "include", "");

    if (cstr_isempty(list->opt_include))
        return false;

    return true;
}

bool mlist_readFile(MovList *list, const char *filepath, const char *drive)
{
    print(" read  : %s", filepath);

    CFileAuto *file = cfile_new();

    if (!cfile_read(file, filepath))
    {
        print("*** Can't open input file: %s", filepath);
        return false;
    }

    CStringAuto *fname = cstr_new_size(16);
    cstr_copy(fname, drive);
    cstr_append(fname, SEP_TAB);

    // Parse all lines.
    CStringAuto *line = cstr_new_size(512);
    int count = 0;

    while (cfile_getline(file, line))
    {
        if (count == 0)
        {
            if (!cstr_startswith(line, DEFAULT_HEADER, true))
            {
                print("*** Invalid header.");

                return false;
            }

            ++count;
            continue;
        }

        if (cstr_isempty(line))
            continue;

        if (!cstr_startswith(line, c_str(fname), true))
            continue;

        MovEntry *entry = mentry_new();

        if (!mentry_readline(entry, c_str(line)))
        {
            print("*** Invalid line in %s", filepath);

            mentry_free(entry);

            return false;
        }

        clist_append(list->entryList, entry);

        ++count;
    }

    print("         %i entries", clist_size(list->entryList));

    return true;
}

bool _mlist_read_directory(MovList *list, MovList *movlist,
                           CString *basedir, CString *subdir, const char *drivename)
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

    CStringAuto *temp = cstr_new_size(64);

    while (cdirparser_read(dir, filepath, NULL))
    {
        if (!cstr_endswith(filepath, ".avi", false)
            && !cstr_endswith(filepath, ".divx", false)
            && !cstr_endswith(filepath, ".flv", false)
            && !cstr_endswith(filepath, ".m2ts", false)
            && !cstr_endswith(filepath, ".m4a", false)
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
        if (fsize < (uint64_t) (list->opt_minsize * 1000000))
            continue;

        if (count < 1)
            print("");

        print("   %s", c_str(filepath));
        ++count;

        MovEntry *entry = mentry_new();

        cstr_copy(entry->drive, drivename);
        cstr_copy(entry->directory, c_str(subdir));
        entry->fsize = fsize;
        entry->ftime = cfileinfo_mtime(fileinfo);

        const char *p = path_ext(c_str(filepath), true);
        if (p)
        {
            ++p;
            cstr_copy(entry->fext, p);
        }

        p = path_basename_ptr(c_str(filepath));
        if (!p)
            p = c_str(filepath);

        cstr_copy(temp, p);
        path_strip_ext(temp, true);
        p = c_str(temp);

        int pos = strstr(p, " - ") - p;

        if (pos == 4 && strlen(p) > 7)
        {
            cstr_left(temp, entry->year, 4);
            cstr_mid(temp, entry->title, (pos + 3), -1);

            //print("year : %s", entry->year.c_str());
            //print("title : %s", entry->title.c_str());
        }
        else
        {
            cstr_copy(entry->title, p);
        }

        cstr_copy(entry->titleKey, c_str(entry->title));
        cstr_copy(entry->sortKey, c_str(entry->titleKey));
        cstr_append(entry->sortKey, c_str(entry->year));

        if (list->opt_media)
        {
            MovEntry *found = mlist_find(movlist, entry);
            if (found)
            {
                mentry_get_media_info(entry, found);
            }

            if (cstr_isempty(entry->mediainfo))
            {
                print("    > read media infos...\n");

                char *cstr = getMediaInfo(c_str(filepath));

                if (!cstr)
                {
                    print("*** error: read media error");
                }
                else
                {
                    cstr_copy(entry->mediainfo, cstr);
                    free(cstr);

                    //print(entry->mediainfo);
                }
            }
        }

        clist_append(list->entryList, entry);
    }

    return true;
}

bool mlist_writeFile(MovList *list, const char *filepath)
{
    CFileAuto *file = cfile_new();

    if (!cfile_open(file, filepath, "wb"))
        return false;

    bool report = false;
    if (str_endswith(filepath, REPORT_EXT, true))
        report = true;

    _mlist_writeHeader(list, file);

    int size = clist_size(list->entryList);

    CStringAuto *temp = cstr_new_size(12);

    for (int i = 0; i < size; ++i)
    {
        MovEntry *entry = (MovEntry*) clist_at(list->entryList, i);

        cfile_write(file, c_str(entry->drive));
        cfile_write(file, SEP_TAB);
        cfile_write(file, c_str(entry->directory));
        cfile_write(file, SEP_TAB);
        cfile_write(file, c_str(entry->year));
        cfile_write(file, SEP_TAB);
        cfile_write(file, c_str(entry->title));
        cfile_write(file, SEP_TAB);
        cfile_write(file, c_str(entry->fext));

        if (report)
        {
            cfile_write(file, SEP_TAB);
            cstr_uint64(temp, (entry->fsize / 1000000));
            cfile_write(file, c_str(temp));

            cfile_write(file, SEP_TAB);
            time_t ltime = entry->ftime / 1000;
            struct tm *mytime = localtime(&ltime);

            cfile_writefmt(file, "%i/%.2i/%.2i-%.2i:%.2i",
                           mytime->tm_year + 1900, mytime->tm_mon + 1, mytime->tm_mday,
                           mytime->tm_hour, mytime->tm_min);
        }
        else
        {
            cfile_write(file, SEP_TAB);
            cstr_uint64(temp, entry->fsize);
            cfile_write(file, c_str(temp));

            cfile_write(file, SEP_TAB);
            cstr_uint64(temp, entry->ftime);
            cfile_write(file, c_str(temp));
        }

        if (list->opt_media)
        {
            cfile_write(file, SEP_TAB);
            cfile_write(file, c_str(entry->mediainfo));
        }

        cfile_write(file, "\n");
    }

    return true;
}

void _mlist_writeHeader(MovList *list, CFile *file)
{
    cfile_write(file, DEFAULT_HEADER);

    if (list->opt_media)
    {
        cfile_write(file, SEP_TAB);
        cfile_write(file, MEDIA_HEADER);
    }

    cfile_write(file, "\n");
}


