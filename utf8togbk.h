#ifndef __UTF8TOGBK_H
#define __UTF8TOGBK_H

#include "string.h"

int SwitchToGbk(const unsigned char* pszBufIn, int nBufInLen, unsigned char* pszBufOut, int* pnBufOutLen);

#endif