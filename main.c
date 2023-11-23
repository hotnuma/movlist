#include "movlist.h"
#include <locale.h>

#define RETURN_ERROR 1

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    int ret = EXIT_SUCCESS;

    MovList *movlist = mlist_new();

    if (!mlist_execute(movlist, argc, argv))
        ret = EXIT_FAILURE;

    mlist_free(movlist);

    return ret;
}


