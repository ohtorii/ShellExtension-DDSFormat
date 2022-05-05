#include "pch.h"
#include "Resource/resource.h"
#include "DDSFormatColExt.h"
#include "SpinLock.h"


namespace {

    class OleStrWrapper {
    public:
        OleStrWrapper(const OleStrWrapper&) = delete;
        OleStrWrapper& operator=(const OleStrWrapper&) = delete;

        OleStrWrapper() : m_olestr(nullptr) {}
        ~OleStrWrapper() { Free(); }
        LPOLESTR& Raw() { return m_olestr; };
    private:
        void Free() {
            CoTaskMemFree(m_olestr); m_olestr = nullptr;
        }
        LPOLESTR    m_olestr;
    };


    class ThreadSafeComModule {
    public:
        ThreadSafeComModule(const ThreadSafeComModule&) = delete;
        ThreadSafeComModule& operator=(const ThreadSafeComModule&) = delete;

        ThreadSafeComModule() {};
        CComModule              m_module;
        int GetLockCount() {
            dds_format::Monitor monitor(m_lock);
            return m_module.GetLockCount();
        }
        HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
            return m_module.GetClassObject(rclsid, riid, ppv);
        }
    private:
        dds_format::SpinLock    m_lock;
    };
    ThreadSafeComModule sg_Module;


    BEGIN_OBJECT_MAP(ObjectMap)
        OBJECT_ENTRY(CLSID_DDSFormatColExt, dds_format::CDDSFormatColExt)
    END_OBJECT_MAP()


    auto const sg_RegistorPath = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
}

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        sg_Module.m_module.Init(ObjectMap, hInstance, &LIBID_DDSFORMATLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        sg_Module.m_module.Term();
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    return (sg_Module.GetLockCount() == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return sg_Module.GetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
    OleStrWrapper olestr;
    {
        auto result = StringFromCLSID(CLSID_DDSFormatColExt, &olestr.Raw());
        if (result != S_OK) {
            return result;
        }
    }
    {
        CRegKey reg;
        {
            auto ret = reg.Open(HKEY_LOCAL_MACHINE, sg_RegistorPath, KEY_SET_VALUE);
            if (ERROR_SUCCESS != ret) {
                return ret;
            }
        }
        {
            auto ret = reg.SetValue(_T("DDS Format column extension"), olestr.Raw());
            reg.Close();
            if (ERROR_SUCCESS != ret) {
                return ret;
            }
        }
    }
    return sg_Module.m_module.RegisterServer(false);
}

STDAPI DllUnregisterServer()
{
    OleStrWrapper olestr;
    {
        auto result = StringFromCLSID(CLSID_DDSFormatColExt, &olestr.Raw());
        if (result != S_OK) {
            return result;
        }
    }
    {
        CRegKey reg;
        auto    lRet = reg.Open(HKEY_LOCAL_MACHINE, sg_RegistorPath, KEY_SET_VALUE);
        if (ERROR_SUCCESS == lRet)
        {
            reg.DeleteValue(olestr.Raw());
            reg.Close();
        }
    }
    return sg_Module.m_module.UnregisterServer(false);
}
