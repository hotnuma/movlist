#include "MovList.h"

//#include <CFileInfo.h>
//#include <sys/stat.h>
//#include <math.h>
//#include <time.h>

#include <locale.h>

#include <print.h>

#define RETURN_ERROR 1

int main(int argc, char **argv)
{
    setlocale(LC_ALL,"fr_FR.UTF-8");

    MovList movlist;

    if (!movlist.execute(argc, argv))
        return RETURN_ERROR;

    return 0;
}


