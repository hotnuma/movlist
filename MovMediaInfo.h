#ifndef MEDIAINFO_H
#define MEDIAINFO_H

#include "CString.h"

void getMediaHeader(CString &result);
bool getMediaInfo(const CString &filepath, CString &result);

#endif // MEDIAINFO_H


