#pragma once

#include <atlbase.h>

class CAppAtlModule :  public CComModule
{
public:
	CAppAtlModule(void);
	~CAppAtlModule(void);

    LONG Unlock();
    DWORD dwThreadID;
    HANDLE hEventShutdown;
    void MonitorShutdown();
    bool StartMonitor();
    bool bActivity;
};
extern CAppAtlModule _Module;

