#pragma once
#ifndef	DDSFORMAT_GENERATEDCODE_H
#define	DDSFORMAT_GENERATEDCODE_H

#if defined(_M_X64)
    #if defined(_DEBUG)
        #include "x64/Debug/DDSFormatCOM.h"
    #else
        #include "x64/Release/DDSFormatCOM.h"
    #endif
#else
    #if defined(_DEBUG)
            #include "Win32/Debug/DDSFormatCOM.h"
        #else
            #include "Win32/Release/DDSFormatCOM.h"
        #endif
    #endif
#endif /* DDSFORMAT_GENERATEDCODE_H */
