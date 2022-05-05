#include "pch.h"
#include "DDSFormatColExt.h"
#include "DDSLoader.h"
#include <cstring>
#include <cassert>
#include <algorithm>


namespace {
    enum class Column{
        ForCC=0,
        MipMapCount,
        Reserved1AsAsciiDump,
        Reserved1AsHexDump,

        Number,
    };
};

namespace dds_format {
    STDMETHODIMP CDDSFormatColExt::Initialize(/* [[maybe_unused]] */ LPCSHCOLUMNINIT psci) {
        (void)psci;
        return S_OK;
    }
    STDMETHODIMP CDDSFormatColExt::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci)
    {
        switch (dwIndex)
        {
        case Column::ForCC:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::ForCC);
            psci-> vt         = VT_LPSTR;
            psci-> fmt        = LVCFMT_LEFT;
            psci-> csFlags    = SHCOLSTATE_TYPE_STR;
            psci-> cChars     = 6;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("FourCC"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("FourCC of DDS"), _TRUNCATE);
            return S_OK;

        case Column::MipMapCount:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::MipMapCount);
            psci-> vt         = VT_INT;
            psci-> fmt        = LVCFMT_RIGHT;
            psci-> csFlags    = SHCOLSTATE_TYPE_INT;
            psci-> cChars     = 2;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("MipMapCount"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("MipMapCount of DDS"), _TRUNCATE);
            return S_OK;

        case Column::Reserved1AsAsciiDump:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::Reserved1AsAsciiDump);
            psci-> vt         = VT_LPSTR;
            psci-> fmt        = LVCFMT_LEFT;
            psci-> csFlags    = SHCOLSTATE_TYPE_STR;
            psci-> cChars     = 16;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("Reserved1(string)"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("DDS reserved1 area as string"), _TRUNCATE);
            return S_OK;

        case Column::Reserved1AsHexDump:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::Reserved1AsHexDump);
            psci-> vt         = VT_LPSTR;
            psci-> fmt        = LVCFMT_LEFT;
            psci-> csFlags    = SHCOLSTATE_TYPE_STR;
            psci-> cChars     = 16;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("Reserved1(Hex)"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("DDS reserved1 area as hex"), _TRUNCATE);
            return S_OK;

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

        std::array<wchar_t,static_cast<size_t>(dds_loader::Loader::MinimumBufferCount::Reserved1HexDump)>      szField;
        szField[0] = '\0';//念のため

        {
            dds_loader::Loader loader;
            if (loader.Load(pscd->wszFile)) {
                switch (pscid->pid)
                {
                case Column::ForCC:
                    loader.GetFourCCAsWChar(szField.data(), szField.size());
                    break;
                case Column::MipMapCount:
                {
                    CComVariant vData(static_cast<int>(loader.GetMipMapCount()));
                    return vData.Detach(pvarData);
                }

                case Column::Reserved1AsAsciiDump:
                    loader.GetReserved1AsAsciiDump(szField.data(), szField.size());
                    break;

                case Column::Reserved1AsHexDump:
                    loader.GetReserved1AsHexDump(szField.data(), szField.size());
                    break;

                default:
                    assert(false);
                    return S_FALSE;
                }
            }
            else {
                //pass
            }
        }

        szField.back() = '\0';//念のため
        CComVariant vData(szField.data());
        return vData.Detach(pvarData);
    }

}; /*namespace dds_format*/

