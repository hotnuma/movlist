#include "Global.h"

#include <CDirParser.h>
#include <libapp.h>
#include <libfile.h>

#include <print.h>

bool getFullPath(const CString &drname, CString &result)
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


