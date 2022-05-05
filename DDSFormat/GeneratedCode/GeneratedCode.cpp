#include"../pch.h"

#if defined(_M_X64)
    #if defined(_DEBUG)
        #include "x64/Debug/DDSFormatCOM_i.c"
    #else
        #include "x64/Release/DDSFormatCOM_i.c"
    #endif
#else
    #if defined(_DEBUG)
        #include "Win32/Debug/DDSFormatCOM_i.c"
    #else
        #include "Win32/Release/DDSFormatCOM_i.c"
    #endif
#endif


