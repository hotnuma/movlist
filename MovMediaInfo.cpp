#include "MovMediaInfo.h"
#include "Global.h"

#include <MediaInfo/MediaInfo.h>

#define MediaInfoNameSpace MediaInfoLib;
using namespace MediaInfoNameSpace

void getMediaHeader(CString &result)
{
    CString sep = SEP_TAB;

    result += "Application";
    result += sep;
    result += "BitRate";
    result += sep;
    result += "Duration";

    result += sep;
    result += "Video";
    result += sep;
    result += "Width";
    result += sep;
    result += "Heigth";
    result += sep;
    result += "Aspect";
    result += sep;
    result += "FrameRate";

    result += sep;
    result += "Audio";
}

bool getMediaInfo(const CString &filepath, CString &result)
{
    size_t wlen = mbstowcs(nullptr, filepath, 0);
    if (wlen < 1)
        return false;

    wchar_t *wstr = wcsalloc(wlen + 1);

    if (mbstowcs(wstr, filepath, wlen + 1) == (size_t) -1)
    {
        free(wstr);
        return false;
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

    //result += CString::fromStdWString(line);

    size_t len = wcstombs(nullptr, line.c_str(), 0);
    if (len < 1)
        return false;

    char *cstr = stralloc(len + 1);

    if (wcstombs(cstr, line.c_str(), len + 1) == (size_t) -1)
    {
        free(cstr);
        return false;
    }

    //result += wcharToCString(line.c_str());

    result += cstr;
    free(cstr);

    return true;
}


