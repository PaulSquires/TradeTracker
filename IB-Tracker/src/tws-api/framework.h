// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <commctrl.h>
#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <algorithm>

#include <objidl.h>
#include <gdiplus.h>


using namespace Gdiplus;
using namespace std::string_literals; // enables 'L' macro

#pragma comment (lib,"Gdiplus.lib")

//' ========================================================================================
//' Simple debug message output to console
//' ========================================================================================
template <typename T>
void dp(T t)
{
    std::wcout << t << std::endl;
}

