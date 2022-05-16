#include "pch.h"
#include "DDSFormatColExt.h"
#include "DDSLoader.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <functional>


namespace {
    enum class Column : DWORD{
        ForCCAsciiDump=0,
        ForCCHexDump,
        PixelFormat,
        Depth,
        RGBBitCount,
        Caps,
        Caps2,
        MipMapCount,
        Reserved1AsAsciiDump,
        Reserved1AsHexDump,
        Number,
    };

    HRESULT InitializeAsString(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        psci-> scid.fmtid = CLSID_DDSFormatColExt;
        psci-> scid.pid   = pid;
        psci-> vt         = VT_LPSTR;
        psci-> fmt        = LVCFMT_LEFT;
        psci-> csFlags    = SHCOLSTATE_TYPE_STR;
        psci-> cChars     = chars;
        wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), title, _TRUNCATE);
        wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), description, _TRUNCATE);
        return S_OK;
    }

    HRESULT InitializeAsInt(SHCOLUMNINFO* psci, DWORD pid, UINT chars, const wchar_t* title, const wchar_t* description) {
        psci-> scid.fmtid = CLSID_DDSFormatColExt;
        psci-> scid.pid   = pid;
        psci-> vt         = VT_INT;
        psci-> fmt        = LVCFMT_RIGHT;
        psci-> csFlags    = SHCOLSTATE_TYPE_INT;
        psci-> cChars     = chars;
        wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), title, _TRUNCATE);
        wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), description, _TRUNCATE);
        return S_OK;
    }

    HRESULT MakeWStr(dds_loader::Loader*loader, std::function<size_t(dds_loader::Loader*, wchar_t*, size_t)> method, VARIANT* pvarData) {
        std::array<wchar_t,static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
        szField[0] = '\0';//念のため
        /*auto _ = */ method(loader, szField.data(), szField.size());
        szField.back() = '\0';//念のため
        CComVariant vData(szField.data());
        return vData.Detach(pvarData);
    }

    HRESULT MakeInt(dds_loader::Loader*loader, std::function<DWORD(dds_loader::Loader*)> method, VARIANT* pvarData) {
        CComVariant vData(static_cast<int>(method(loader)));
        return vData.Detach(pvarData);
    }
};

namespace dds_format {
    STDMETHODIMP CDDSFormatColExt::Initialize([[maybe_unused]] LPCSHCOLUMNINIT psci) {
        return S_OK;
    }

    STDMETHODIMP CDDSFormatColExt::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci)
    {
        switch (dwIndex)
        {
        case static_cast<DWORD>(Column::ForCCAsciiDump):
            return InitializeAsString(psci, dwIndex, 4, _T("FourCC(Ascii)"), _T("FourCC area as ascii dump"));

        case static_cast<DWORD>(Column::ForCCHexDump):
            return InitializeAsString(psci, dwIndex, 4, _T("FourCC(Hex)"), _T("FourCC area as hex dump"));

        case static_cast<DWORD>(Column::PixelFormat):
            return InitializeAsString(psci, dwIndex, 10, _T("PixelFormat"), _T("PixelFormat of DDS"));

        case static_cast<DWORD>(Column::Depth):
            return InitializeAsInt(psci, dwIndex, 2, _T("Depth"), _T("Depth of DDS"));

        case static_cast<DWORD>(Column::RGBBitCount):
            return InitializeAsInt(psci, dwIndex, 2, _T("RGBBitCount"), _T("RGBBitCount of DDS"));

        case static_cast<DWORD>(Column::Caps):
            return InitializeAsString(psci, dwIndex, 10, _T("Caps"), _T("Caps of DDS"));

        case static_cast<DWORD>(Column::Caps2):
            return InitializeAsString(psci, dwIndex, 10, _T("Caps2"), _T("Caps2 of DDS"));

        case static_cast<DWORD>(Column::MipMapCount):
            return InitializeAsInt(psci, dwIndex, 2, _T("MipMapCount"), _T("MipMapCount of DDS"));

        case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
            return InitializeAsString(psci, dwIndex, 16, _T("Reserved1(Ascii)"), _T("DDS reserved1 area as Ascii dump"));

        case static_cast<DWORD>(Column::Reserved1AsHexDump):
            return InitializeAsString(psci, dwIndex, 16, _T("Reserved1(Hex)"), _T("DDS reserved1 area as hex dump"));

        default:
            return S_FALSE;
        }
    }

    STDMETHODIMP CDDSFormatColExt::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData)
    {
        if (pscid->fmtid != CLSID_DDSFormatColExt)
        {
            return S_FALSE;
        }
        if (static_cast<DWORD>(Column::Number) <= pscid->pid) {
            return S_FALSE;
        }
        if (0 != _wcsicmp(pscd->pwszExt, L".dds")) {
            return S_FALSE;
        }
        if (pscd->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_OFFLINE)) {
            return S_FALSE;
        }

        dds_loader::Loader loaderDXT1;
        if (loaderDXT1.Load(pscd->wszFile) == false)
        {
            return S_FALSE;
        }
        switch (pscid->pid)
        {
        case static_cast<DWORD>(Column::ForCCAsciiDump):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetFourCCAsAsciiDump, pvarData);

        case static_cast<DWORD>(Column::ForCCHexDump):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetFourCCAsHexDump, pvarData);

        case static_cast<DWORD>(Column::PixelFormat):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetDDPFFlagsAsWChar, pvarData);

        case static_cast<DWORD>(Column::Depth):
            return MakeInt(&loaderDXT1, &dds_loader::Loader::GetDepth, pvarData);

        case static_cast<DWORD>(Column::RGBBitCount):
            return MakeInt(&loaderDXT1, &dds_loader::Loader::GetRGBBitCount, pvarData);

        case static_cast<DWORD>(Column::Caps):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetCapsAsWChar, pvarData);

        case static_cast<DWORD>(Column::Caps2):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetCaps2AsWChar, pvarData);

        case static_cast<DWORD>(Column::MipMapCount):
            return MakeInt(&loaderDXT1, &dds_loader::Loader::GetMipMapCount, pvarData);

        case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetReserved1AsAsciiDump, pvarData);

        case static_cast<DWORD>(Column::Reserved1AsHexDump):
            return MakeWStr(&loaderDXT1, &dds_loader::Loader::GetReserved1AsHexDump, pvarData);

        [[unlikely]]default:
            assert(false);
            return S_FALSE;
        }
    }

}; /*namespace dds_format*/

