#include "mlist_mediainfo.h"
#include <MediaInfo/MediaInfo.h>

#define MediaInfoNameSpace MediaInfoLib;
using namespace MediaInfoNameSpace

char* getMediaInfo(const char *filepath)
{
    size_t wlen = mbstowcs(nullptr, filepath, 0);
    if (wlen < 1)
        return NULL;

    //wchar_t *wstr = wcsalloc(wlen + 1);

    wchar_t *wstr = (wchar_t*) malloc((wlen + 1) * sizeof(wchar_t));

    if (mbstowcs(wstr, filepath, wlen + 1) == (size_t) -1)
    {
        free(wstr);
        return NULL;
    }

    MediaInfo mediaInfo;
    mediaInfo.Open(wstr);
    free(wstr);

    // Format Duration Format Width Heigth Format
    String line;

    String temp = mediaInfo.Get(Stream_General, 0, __T("Encoded_Application"), Info_Text, Info_Name).c_str();
    if (temp.empty())
        temp = mediaInfo.Get(Stream_General, 0, __T("Encoded_Library"), Info_Text, Info_Name).c_str();
    line += temp;

    line += __T("\t");
    line += mediaInfo.Get(Stream_General, 0, __T("OverallBitRate"), Info_Text, Info_Name).c_str();

    line += __T("\t");
    line += mediaInfo.Get(Stream_General, 0, __T("Duration/String3"), Info_Text, Info_Name).c_str();

    line += __T("\t");
    line += mediaInfo.Get(Stream_Video, 0, __T("Format"), Info_Text, Info_Name).c_str();

    line += __T("\t");
    line += mediaInfo.Get(Stream_Video, 0, __T("Width"), Info_Text, Info_Name).c_str();

    line += __T("\t");
    line += mediaInfo.Get(Stream_Video, 0, __T("Height"), Info_Text, Info_Name).c_str();

    line += __T("\t");

    line += __T("[");
    line += mediaInfo.Get(Stream_Video, 0, __T("DisplayAspectRatio/String"), Info_Text, Info_Name).c_str();
    line += __T("]");

    line += __T("\t");
    line += mediaInfo.Get(Stream_Video, 0, __T("FrameRate"), Info_Text, Info_Name).c_str();

    line += __T("\t");
    temp = mediaInfo.Get(Stream_Audio, 0, __T("Format"), Info_Text, Info_Name).c_str();
    if (temp == __T("MPEG Audio"))
    {
        String profile = mediaInfo.Get(Stream_Audio, 0, __T("Format_Profile"), Info_Text, Info_Name).c_str();
        if (profile == __T("Layer 3"))
            line += __T("Mp3");
        else
            line += temp;
    }
    else
    {
        line += temp;
    }

    mediaInfo.Close();

    size_t len = wcstombs(nullptr, line.c_str(), 0);
    if (len < 1)
        return NULL;

    char *cstr = (char*) malloc((len + 1) * sizeof(char)); //stralloc(len + 1);

    if (wcstombs(cstr, line.c_str(), len + 1) == (size_t) -1)
    {
        free(cstr);
        return NULL;
    }

    return cstr;
}


