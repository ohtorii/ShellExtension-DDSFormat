#pragma once
#if !defined(AFX_STDAFX_H__AA73F335_8B55_42E1_88C7_34C055E47506__INCLUDED_)
#define      AFX_STDAFX_H__AA73F335_8B55_42E1_88C7_34C055E47506__INCLUDED_

//#define _ATL_APARTMENT_THREADED

// ATL
#include <atlbase.h>
#include <atlcom.h>
#include <atlconv.h>


// Win32
#include <shlwapi.h>
#include <comdef.h>
#include <shlobj.h>
#include <shlguid.h>
#include <initguid.h>

#ifdef DEFAULT_UNREACHABLE
#undef DEFAULT_UNREACHABLE
#endif
#ifndef _DEBUG
    #if _MSC_VER > 1100
        #define DEFAULT_UNREACHABLE default: __assume(0); break
    #else
        #define DEFAULT_UNREACHABLE default: break
    #endif
#elif defined(ASSERT)
#define DEFAULT_UNREACHABLE default: ASSERT(0); break   // use MFC assert
#else
#define DEFAULT_UNREACHABLE default: _ASSERTE(0); break // use CRT assert
#endif



#endif
