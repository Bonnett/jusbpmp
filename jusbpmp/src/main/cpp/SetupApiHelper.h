#pragma once

#include <dbt.h>
#include <Setupapi.h>
#include <cfgmgr32.h>
#include <winioctl.h>

class CDeviceInfo;

class SetupApiHelper{

	#define WPD_DEVINTERFACE		_T("{6ac27878-a6fa-4155-ba85-f98f491d4f33}")
	#define ARCHOS_VID				_T("vid_0e79")

public:
	static BOOL			CheckDevice(DWORD unitMask, CDeviceInfo* pInfo);
	static BOOL			CheckDevice(CString drive, CDeviceInfo* pInfo);
	static int			DetectDevices(CMapStringToPtr* devlist);
	static int			DetectDevicesSetupApi(CMapStringToPtr* devlist);
	static CString		GetMountPointForDevice(DWORD deviceNumber);
	static void			GetDrivesFromMask(DWORD unitMask, CStringList *drivelist);

protected:
	static DEVINST		GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType);
	static BOOL			GetDeviceInfoEx(CString strDosDevicePath, CDeviceInfo* pInfo);
	static CONFIGRET	GetDeviceName(DEVNODE DevNode, CString& devName);
	static CONFIGRET	GetDeviceManuf(DEVNODE DevNode, CString& devManuf);

};