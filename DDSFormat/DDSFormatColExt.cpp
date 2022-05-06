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

    //typedef size_t(dds_loader::Loader::*WCstrMethod)(wchar_t*,size_t);
    HRESULT MakeWStr(dds_loader::Loader*loader, std::function<size_t(dds_loader::Loader*, wchar_t*, size_t)> method, VARIANT* pvarData) {
        std::array<wchar_t,static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
        szField[0] = '\0';//念のため
        /*auto _ = */ method(loader, szField.data(), szField.size());
        szField.back() = '\0';//念のため
        CComVariant vData(szField.data());
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
            return InitializeAsString(psci, static_cast<DWORD>(Column::ForCCAsciiDump), 4, _T("FourCC(Ascii)"), _T("FourCC area as ascii dump"));

        case static_cast<DWORD>(Column::ForCCHexDump):
            return InitializeAsString(psci, static_cast<DWORD>(Column::ForCCHexDump), 4, _T("FourCC(Hex)"), _T("FourCC area as hex dump"));

        case static_cast<DWORD>(Column::PixelFormat):
            return InitializeAsString(psci, static_cast<DWORD>(Column::PixelFormat), 10, _T("PixelFormat"), _T("PixelFormat of DDS"));

        case static_cast<DWORD>(Column::MipMapCount):
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::MipMapCount);
            psci-> vt         = VT_INT;
            psci-> fmt        = LVCFMT_RIGHT;
            psci-> csFlags    = SHCOLSTATE_TYPE_INT;
            psci-> cChars     = 2;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("MipMapCount"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("MipMapCount of DDS"), _TRUNCATE);
            return S_OK;

        case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
            return InitializeAsString(psci, static_cast<DWORD>(Column::Reserved1AsAsciiDump), 16, _T("Reserved1(Ascii)"), _T("DDS reserved1 area as Ascii dump"));

        case static_cast<DWORD>(Column::Reserved1AsHexDump):
            return InitializeAsString(psci, static_cast<DWORD>(Column::Reserved1AsHexDump), 16, _T("Reserved1(Hex)"), _T("DDS reserved1 area as hex dump"));

        default:
            assert(false);
            return S_FALSE;
        }
    }

    STDMETHODIMP CDDSFormatColExt::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT* pvarData)
    {
        USES_CONVERSION;

        if (pscid->fmtid != CLSID_DDSFormatColExt)
        {
            return S_FALSE;
        }
        if (static_cast<DWORD>(Column::Number) <= pscid->pid) {
            return S_FALSE;
        }

        if (pscd->dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_OFFLINE)) {
            return S_FALSE;
        }

        if (0 != _wcsicmp(pscd->pwszExt, L".dds")) {
            return S_FALSE;
        }

        {
             using namespace std::placeholders;

            dds_loader::Loader loader;
            if (loader.Load(pscd->wszFile)) {
                switch (pscid->pid)
                {
                case static_cast<DWORD>(Column::ForCCAsciiDump):
                    return MakeWStr(&loader, &dds_loader::Loader::GetFourCCAsAsciiDump, pvarData);

                case static_cast<DWORD>(Column::ForCCHexDump):
                    return MakeWStr(&loader, &dds_loader::Loader::GetFourCCAsHexDump, pvarData);

                case static_cast<DWORD>(Column::PixelFormat):
                    return MakeWStr(&loader, &dds_loader::Loader::GetDDPFFlags, pvarData);

                case static_cast<DWORD>(Column::MipMapCount):
                {
                    CComVariant vData(static_cast<int>(loader.GetMipMapCount()));
                    return vData.Detach(pvarData);
                }

                case static_cast<DWORD>(Column::Reserved1AsAsciiDump):
                    return MakeWStr(&loader, &dds_loader::Loader::GetReserved1AsAsciiDump, pvarData);

                case static_cast<DWORD>(Column::Reserved1AsHexDump):
                    return MakeWStr(&loader, &dds_loader::Loader::GetReserved1AsHexDump, pvarData);

                [[unlikely]]default:
                    assert(false);
                    return S_FALSE;
                }
            }
            else {
                //pass
            }
        }

        return S_FALSE;
    }

}; /*namespace dds_format*/

