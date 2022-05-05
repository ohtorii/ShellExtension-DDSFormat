#include "pch.h"
#include "DDSFormatColExt.h"
#include "DDSLoader.h"
#include <cstring>
#include <algorithm>


namespace {
    enum class Column{
        ForCC=0,
        Reserved1AsString,
        Reserved1AsHex,
        Number,
    };
};

namespace dds_format {
    STDMETHODIMP CDDSFormatColExt::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO* psci)
    {
        if (static_cast<DWORD>(Column::Number)  <= dwIndex) {
            return S_FALSE;
        }

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
            break;

        case Column::Reserved1AsString:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::Reserved1AsString);
            psci-> vt         = VT_LPSTR;
            psci-> fmt        = LVCFMT_LEFT;
            psci-> csFlags    = SHCOLSTATE_TYPE_STR;
            psci-> cChars     = 16;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("Reserved1(string)"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("DDS reserved1 area as string"), _TRUNCATE);
            break;

        case Column::Reserved1AsHex:
            psci-> scid.fmtid = CLSID_DDSFormatColExt;
            psci-> scid.pid   = static_cast<DWORD>(Column::Reserved1AsHex);
            psci-> vt         = VT_LPSTR;
            psci-> fmt        = LVCFMT_LEFT;
            psci-> csFlags    = SHCOLSTATE_TYPE_STR;
            psci-> cChars     = 16;
            wcsncpy_s(psci->wszTitle, _countof(psci->wszTitle), _T("Reserved1(Hex)"), _TRUNCATE);
            wcsncpy_s(psci->wszDescription, _countof(psci->wszDescription), _T("DDS reserved1 area as hex"), _TRUNCATE);
            break;

            DEFAULT_UNREACHABLE;
        }

        return S_OK;
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

        std::array<wchar_t,dds_loader::Loader::MinimumReserved1Count>      szField;
        static_assert(dds_loader::Loader::MinimumFourCCCount < szField.size(),"The buffer size is small.");
        szField[0] = '\0';//念のため

        {
            dds_loader::Loader loader;
            if (loader.Load(pscd->wszFile)) {
                switch (pscid->pid)
                {
                case Column::ForCC:
                    loader.GetFourCCAsWChar(szField.data(), szField.size());
                    break;

                case Column::Reserved1AsString:
                    loader.GetReserved1AsWChar(szField.data(), szField.size());
                    break;

                case Column::Reserved1AsHex:
                    loader.GetReserved1AsWCharDump(szField.data(), szField.size());
                    break;

                DEFAULT_UNREACHABLE;
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

