#include "mlist_global.h"

#include <cdirparser.h>
#include <libapp.h>

#include <print.h>

bool get_fullpath(CString *result, const char *drname)
{
    result.clear();

    CString user = getUserName();
    if (user.isEmpty())
    {
        print("*** error: can't get user name");
        return false;
    }

    CString media = strFmt("/media/%s", user.c_str());

    CDirParser dir;
    if (!dir.open(media, CDP_DIRS))
    {
        print("*** error: can't parse media directory");
        return false;
    }

    CString dirpath;

    while (dir.read(dirpath))
    {
        //print(dirpath);

        CString fullpath = strFmt("%s/%s.ini",
                                  dirpath.c_str(),
                                  drname.c_str());

        //print("path : %s", fullpath.c_str());

        if (fileExists(fullpath))
        {
            result = fullpath;

            return true;
        }
    }

    return false;
}


