#include "mlist_global.h"

#include <cdirparser.h>
#include <cfile.h>
#include <libapp.h>

#include <print.h>

bool get_fullpath(CString *result, const char *drname)
{
    cstr_clear(result);

    CStringAuto *user = cstr_new_size(32);
    get_username(user);

    if (cstr_isempty(user))
    {
        print("*** error: can't get user name");
        return false;
    }

    CStringAuto *media = cstr_new_size(64);
    cstr_fmt(media, "/media/%s", c_str(user));

    CDirParserAuto *dir = cdirparser_new();
    if (!cdirparser_open(dir, c_str(media), CDP_DIRS))
    {
        print("*** error: can't parse media directory");
        return false;
    }

    CStringAuto *dirpath = cstr_new_size(128);
    CStringAuto *fullpath = cstr_new_size(128);

    while (cdirparser_read(dir, dirpath, NULL))
    {
        cstr_fmt(fullpath, "%s/%s.ini", c_str(dirpath), drname);

        //print("path : %s", c_str(fullpath));

        if (file_exists(c_str(fullpath)))
        {
            cstr_copy(result, c_str(fullpath));

            return true;
        }
    }

    return false;
}


