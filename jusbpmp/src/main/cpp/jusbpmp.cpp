// jusbpmp.cpp : définit le point d'entrée pour l'application DLL.
//

#include "stdafx.h"
#include "AppAtlModule.h"

#include "transfernotifier_h.h"
#include "transfernotifier_i.c"

CAppAtlModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		HRESULT hr = S_OK;
		CoInitializeEx(NULL, COINIT_MULTITHREADED);

		// initialize COM module to register progress interface
		// FIXME what if error?
		hr = _Module.Init (ObjectMap, hModule, &LIBID_JusbPmpComLib);
		hr = _Module.RegisterClassObjects(CLSCTX_INPROC_SERVER,REGCLS_MULTIPLEUSE);
		hr = _Module.RegisterServer(TRUE);
		DisableThreadLibraryCalls(hModule);
	}else 
		if (ul_reason_for_call == DLL_PROCESS_DETACH) {
			// remove COM classes from registry
			HRESULT hr = _Module.RevokeClassObjects();
			hr = _Module.UnregisterServer(TRUE);
			_Module.Term();

			// uninitialize COM
			CoUninitialize();
		}
   return TRUE;

	
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

