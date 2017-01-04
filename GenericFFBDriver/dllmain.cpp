// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

long g_cRefThisDll;
HANDLE g_module;

#include "ClassFactory.h"
#include "Registrar.h"
#include "FFBDriver.h"

long * CObjRoot::p_ObjCount = NULL; // this is just because i didnt want to use any globals inside the
									// class framework.

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_module = hModule;
		CObjRoot::p_ObjCount = &g_cRefThisDll;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
	*ppvOut = NULL;
	if (IsEqualIID(rclsid, CLSID_FFBDriver))
	{
		// declare a classfactory for CmyInterface class 
		CClassFactory<FFBDriver> *pcf = new CClassFactory<FFBDriver>;
		return pcf->QueryInterface(riid, ppvOut);
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI  DllCanUnloadNow(void)
{
	return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllRegisterServer(void)
{
	CDllRegistrar registrar;  // this class should create standard entries in registry 
	CHAR path[MAX_PATH];
	GetModuleFileNameA((HMODULE)g_module, path, MAX_PATH);
	return registrar.RegisterObject(CLSID_FFBDriver, "GenericFFBDriver", "FFBDriver", path) ? S_OK : S_FALSE;
}

STDAPI DllUnregisterServer(void)
{
	CDllRegistrar registrar;
	return registrar.UnRegisterObject(CLSID_FFBDriver, "GenericFFBDriver", "FFBDriver") ? S_OK : S_FALSE;
}

