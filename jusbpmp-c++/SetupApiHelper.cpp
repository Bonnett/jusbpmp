#include "stdafx.h"
#include "SetupApiHelper.h"
#include <math.h>




DEVINST SetupApiHelper::GetDrivesDevInstByDeviceNumber(long DeviceNumber, UINT DriveType){

	GUID* guid;

	switch (DriveType) {
	case DRIVE_REMOVABLE:
	case DRIVE_FIXED:
		guid = (GUID*)&GUID_DEVINTERFACE_DISK;
		break;
	default:
		return 0;
	}

	// Get device interface info set handle for all devices attached to system
	HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE)	{
		return 0;
	}

	// Retrieve a context structure for a device interface of a device information set
	DWORD dwIndex = 0;
	long res;

	BYTE Buf[1024];
	PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Buf;
	SP_DEVICE_INTERFACE_DATA         spdid;
	SP_DEVINFO_DATA                  spdd;
	DWORD                            dwSize;
	
	spdid.cbSize = sizeof(spdid);

	while ( true )	{
		res = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex, &spdid);
		if ( !res ) {
			break;
		}

		dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL); // check the buffer size

		if ( dwSize!=0 && dwSize<=sizeof(Buf) ) {

			pspdidd->cbSize = sizeof(*pspdidd); // 5 Bytes!

			ZeroMemory(&spdd, sizeof(spdd));
			spdd.cbSize = sizeof(spdd);

			long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd, dwSize, &dwSize, &spdd);
			if ( res ) {

				// in case you are interested in the USB serial number:
				// the device id string contains the serial number if the device has one,
				// otherwise a generated id that contains the '&' char...
				/*
				DEVINST DevInstParent = 0;
				CM_Get_Parent(&DevInstParent, spdd.DevInst, 0); 
				char szDeviceIdString[MAX_PATH];
				CM_Get_Device_ID(DevInstParent, szDeviceIdString, MAX_PATH, 0);
				printf("DeviceId=%s\n", szDeviceIdString);
				*/

				// open the disk or cdrom or floppy
				HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
				if ( hDrive != INVALID_HANDLE_VALUE ) {
					// get its device number
					STORAGE_DEVICE_NUMBER sdn;
					DWORD dwBytesReturned = 0;
					res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
					if ( res ) {
						TRACE(_T("Device number %d\n"),sdn.DeviceNumber);
						if ( DeviceNumber == (long)sdn.DeviceNumber ) {  // match the given device number with the one of the current device

							CloseHandle(hDrive);
							SetupDiDestroyDeviceInfoList(hDevInfo);
							return spdd.DevInst;
						}
					}
					CloseHandle(hDrive);
				}
			}
		}
		dwIndex++;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}

BOOL SetupApiHelper::CheckDevice(CString drive, CDeviceInfo* pInfo){
	BOOL		bReturnCode = FALSE;
	DEVINST		devinst;
	DEVINST		devinstparent;
	CONFIGRET	cr;
	TCHAR		szVolname[MAX_PATH];

	CString devpath;
	devpath.Format(_T("\\\\.\\%s:"),drive);

	CString voldevpath;
	voldevpath.Format(_T("%s:\\"),drive);

	ZeroMemory(szVolname,MAX_PATH);

	GetVolumeNameForVolumeMountPoint(voldevpath,szVolname,MAX_PATH);
	TRACE(_T("Volume Arrival %s\n"),szVolname);

	CString dosdevpath;
	dosdevpath.Format(_T("%s:"),drive);
	
	// check for id.sys
	CFileStatus fs;
	CString idsyspath = dosdevpath;
	idsyspath += _T("\\System\\id.sys");
	if (!CFile::GetStatus(idsyspath,fs)){
			return FALSE;
	}

	ZeroMemory(szVolname,MAX_PATH);
	QueryDosDevice(dosdevpath,szVolname,MAX_PATH);
	TRACE(_T("Volume Arrival %s\n"),szVolname);


	HANDLE hVolume = INVALID_HANDLE_VALUE;
	hVolume = CreateFile(devpath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (hVolume != INVALID_HANDLE_VALUE) {
		
		STORAGE_DEVICE_NUMBER sdn;
		DWORD dwBytesReturned = 0;
		long DeviceNumber = -1;
		long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
		CloseHandle(hVolume);
		if ( res ) {

			CString strCanonical;
			CString strUsbId;
			CString devname;

			DeviceNumber = sdn.DeviceNumber;
		

			devinst = GetDrivesDevInstByDeviceNumber(DeviceNumber,DRIVE_REMOVABLE);

			ULONG devSize;
			TCHAR szDevinstId[MAX_PATH];
			ZeroMemory(szDevinstId,MAX_PATH);

			cr = CM_Get_Device_ID_Size(&devSize,devinst,0);
			cr = CM_Get_Device_ID(devinst,szDevinstId,devSize,0);

			TRACE(_T("Volume Arrival %s\n"),szDevinstId);
			// FIXME this is canonical
			strCanonical = szDevinstId;
			strCanonical.MakeLower();


			cr = CM_Get_Parent(&devinstparent,devinst,0);
			ZeroMemory(szDevinstId,MAX_PATH);

			cr = CM_Get_Device_ID_Size(&devSize,devinstparent,0);
			cr = CM_Get_Device_ID(devinstparent,szDevinstId,devSize,0);

			cr = GetDeviceName(devinstparent, devname);
		
			TRACE(_T("Volume Arrival %s\n"),szDevinstId);
			// FIXME this is usb ID
			strUsbId = szDevinstId;
			strUsbId.MakeLower();

			if (strUsbId.Find(ARCHOS_VID) != -1){
				CString pid = HelperLib::GetProductIdFromString(strUsbId);
				CString serial = HelperLib::GetDeviceIdFromString(strUsbId, _T("\\"));


				pInfo->m_strProductId = pid;
				pInfo->m_strPkey = serial;						
				pInfo->m_strCanonical = strCanonical;
				pInfo->m_strName = devname;
				pInfo->m_strMountPath = dosdevpath;
				pInfo->m_strUSBLayerId = strUsbId;

				BOOL bInfoEx = GetDeviceInfoEx(dosdevpath,pInfo);
				
				// free space
				ULARGE_INTEGER ulFreeBytes;
				BOOL bRetDisk = GetDiskFreeSpaceEx(dosdevpath, &ulFreeBytes, NULL, NULL);
				if (bRetDisk){
					double dblFree = HelperLib::LargeIntToDouble(&ulFreeBytes);
					// limit should be in MB
					double megabit = pow(1024.0,2);
					pInfo->m_dwMemFreeKB = ( dblFree / 1024 );
				}


				bReturnCode = TRUE;
				return bReturnCode;

			}

		}
	}
	return bReturnCode;
}

BOOL SetupApiHelper::CheckDevice(DWORD unitMask, CDeviceInfo* pInfo){
	
	BOOL		bReturnCode = FALSE;
	DWORD		dwNewMask;

	dwNewMask = unitMask;
	for (int i=0;i<32;i++){
		if ( (dwNewMask & 1) == 1) {
			TCHAR drive = 'A' + i;
			TRACE("Volume Arrival %c\n",drive);
			CString driveLetter;
			driveLetter.Empty();
			driveLetter += drive;
			bReturnCode = CheckDevice(driveLetter,pInfo);
			if (bReturnCode){
				return bReturnCode;
			}			
		}
		dwNewMask = (dwNewMask >> 1);
	}

	return bReturnCode;
}


BOOL SetupApiHelper::GetDeviceInfoEx(CString strDosDevicePath, CDeviceInfo* pInfo){
	
	CString possibleVersion;
	CString possiblePkey;
	BOOL bResult = TRUE;
	
	// read and parse id.sys
	CString fpth = strDosDevicePath;
	fpth += _T("\\System\\id.sys");


	CParserIdSys parser(fpth);
	bResult = parser.ParseIdSys();

	pInfo->m_strStorageGB.Format(_T("%d"), parser.GetStoragegbData());
	pInfo->m_strFullPkey = parser.GetPkey();
	pInfo->m_strDeviceVersion = parser.GetVersion();
	pInfo->m_strMacWifi = parser.GetMacWifi();
	pInfo->m_strMacEth = parser.GetMacEth();
	pInfo->m_strImei = parser.GetImei();

	return bResult;

/*

	CFile f;
	BOOL b = f.Open(fpth, CFile::modeRead, NULL);
	if (!b) return FALSE;

	CStringA asciiBuf;
	char szBuf[50];
	ZeroMemory(szBuf,50);
	UINT nBytes = f.Read(szBuf,6);
	if (nBytes != 6) {
		f.Close();
		return FALSE;
	}
	asciiBuf = szBuf;
	TRACE(_T("read : %s, should be ARCHOS\n"),CA2T(asciiBuf));

	ZeroMemory(szBuf,50);
	nBytes = f.Read(szBuf,14);
	if (nBytes != 14) {
		f.Close();
		return FALSE;
	}
	asciiBuf = szBuf;
	possiblePkey = CA2T(asciiBuf);
	TRACE(_T("read : %s, should be PKEY\n"),possiblePkey);

	unsigned __int32 magic = 0;
	nBytes = f.Read(&magic,sizeof(unsigned __int32));
	if (nBytes != sizeof(unsigned __int32)) {
		f.Close();
		return FALSE;
	}

	if (magic != 0x53524556){
		pInfo->m_strDeviceVersion = _T("1.8.00");
		pInfo->m_strFullPkey = possiblePkey;
		f.Close();
		return TRUE;
	}

	unsigned __int32 versionSize = 0;

	nBytes = f.Read(&versionSize,sizeof(unsigned __int32));
	if (nBytes != sizeof(unsigned __int32)) {
		f.Close();
		return E_FAIL;
	}

	ZeroMemory(szBuf,50);
	nBytes = f.Read(szBuf,versionSize);
	if (nBytes != versionSize) {
		f.Close();
		return E_FAIL;
	}
	asciiBuf = szBuf;
	asciiBuf = asciiBuf.Trim();
	possibleVersion = CA2T(asciiBuf);
	TRACE(_T("read : %s, should be Version\n"),possibleVersion);
	
	pInfo->m_strDeviceVersion = possibleVersion;
	pInfo->m_strFullPkey = possiblePkey;
	

	f.Close();
	return TRUE;
	*/
}

CONFIGRET SetupApiHelper::GetDeviceName(DEVNODE DevNode, CString& devName){
	CONFIGRET cr = 0;
	CString	strType;
    CString	strValue;
    LPTSTR	lpszBuffer;

	devName.Empty();
    
	int  nBufferSize = MAX_PATH + MAX_DEVICE_ID_LEN;
    ULONG  ulBufferLen = nBufferSize * sizeof(TCHAR);
    
	lpszBuffer  = strValue.GetBuffer(nBufferSize);
	cr = CM_Get_DevNode_Registry_Property_Ex(DevNode,
		CM_DRP_LOCATION_INFORMATION, NULL,
		lpszBuffer, &ulBufferLen, 0, NULL);

    if (cr != CR_SUCCESS) {
		ulBufferLen = nBufferSize * sizeof(TCHAR);
		
		cr = CM_Get_DevNode_Registry_Property_Ex(DevNode,
			CM_DRP_DEVICEDESC, NULL,
			lpszBuffer, &ulBufferLen, 0, NULL);

		if (cr != CR_SUCCESS) {
			return cr;
		}
		devName = lpszBuffer;
		return cr;
    } 
	devName = lpszBuffer;
	
	return cr;
}

CONFIGRET SetupApiHelper::GetDeviceManuf(DEVNODE DevNode, CString& devManuf){
	CONFIGRET cr = 0;
	CString	strType;
    CString	strValue;
    LPTSTR	lpszBuffer;

	devManuf.Empty();
    
	int  nBufferSize = MAX_PATH + MAX_DEVICE_ID_LEN;
    ULONG  ulBufferLen = nBufferSize * sizeof(TCHAR);
    
	lpszBuffer  = strValue.GetBuffer(nBufferSize);
	cr = CM_Get_DevNode_Registry_Property_Ex(DevNode,
		CM_DRP_MFG, NULL,
		lpszBuffer, &ulBufferLen, 0, NULL);

 	devManuf = lpszBuffer;
	
	return cr;
}

void SetupApiHelper::GetDrivesFromMask(DWORD unitMask, CStringList *drivelist){
	DWORD		dwNewMask;

	dwNewMask = unitMask;
	for (int i=0;i<32;i++){
		if ( (dwNewMask & 1) == 1) {
			TCHAR drive = 'A' + i;
			TRACE("Volume Removal %c\n",drive);
			CString driveLetter;
			driveLetter.Empty();
			driveLetter += drive;
			drivelist->AddTail(driveLetter)	;
		}
		dwNewMask = (dwNewMask >> 1);
	}

}

int SetupApiHelper::DetectDevices(CMapStringToPtr* devlist){

	int nret = 0;
	DWORD dwDrives = GetLogicalDrives();

	CDeviceInfo* pInfo = new CDeviceInfo();
	if (CheckDevice(dwDrives,pInfo)){
		POSITION pos;
		CDeviceInfo* pit = NULL;
		if ( devlist->Lookup(pInfo->m_strCanonical, (void*&)pit)){
			delete(pInfo);
			pInfo = pit;
		} else {
			(*devlist)[pInfo->m_strCanonical] = pInfo;
		}
		nret = 1;
	}
	return nret;

	

	/*
	DWORD	Chars					= 0;
    WCHAR	DeviceName[MAX_PATH]	= _T("");
    DWORD	Error					= ERROR_SUCCESS;
    HANDLE	FindHandle				= INVALID_HANDLE_VALUE;
    BOOL	Found					= FALSE;
    DWORD	Index					= 0;
    BOOL	Status					= FALSE;
    TCHAR	VolumeName[MAX_PATH]	= _T("");
	HRESULT	hr						= E_FAIL;

	try {

		//
		// Enumerate all volumes in the system
		//
		FindHandle = FindFirstVolume(VolumeName, ARRAYSIZE(VolumeName));

		if (FindHandle == INVALID_HANDLE_VALUE)	{
			Error = GetLastError();
			TRACE(_T("FindFirstVolumeW failed with error code %d\n"), Error);
			throw E_FAIL;
		}

		for (;;)
		{
			//
			// Skip the \\?\ prefix and remove the trailing backslash
			//
			Index = wcslen(VolumeName) - 1;

			if (VolumeName[0]     != L'\\' ||
				VolumeName[1]     != L'\\' ||
				VolumeName[2]     != L'?'  ||
				VolumeName[3]     != L'\\' ||
				VolumeName[Index] != L'\\') 
			{
				Error = ERROR_BAD_PATHNAME;
				TRACE(_T("FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n"), VolumeName);
				throw E_FAIL;
			}

			VolumeName[Index] = _T('\0');

			Chars = QueryDosDevice(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

			VolumeName[Index] = _T('\\');

			if (!Chars) 
			{
				Error = GetLastError();
				TRACE(_T("QueryDosDeviceW failed with error code %d\n"), Error);
				throw E_FAIL;
			}

			CDeviceInfo* pInfo = new CDeviceInfo();
			if (CheckDevice(DeviceName,pInfo)){
				POSITION pos;
				CDeviceInfo* pit = NULL;
				if ( !devlist->Lookup(pInfo->m_strCanonical, (void*&)pit)){
					delete(pInfo);
					pInfo = pit;
				} else {
					(*devlist)[pInfo->m_strCanonical] = pInfo;
				}
				Found = TRUE;
				break;
			}
			delete(pInfo);

			//
			// Move on to the next
			//
			Status = FindNextVolume(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

			if (!Status) 
			{
				Error = GetLastError();

				if (Error != ERROR_NO_MORE_FILES) 
				{
					TRACE(_T("FindNextVolumeW failed with error code %d\n"), Error);

					throw E_FAIL;
				}

				//
				// We've finished iterating
				// through all  the volumes
				//
				Error = ERROR_SUCCESS;
				break;
			}
		}

		FindVolumeClose(FindHandle);
		FindHandle = INVALID_HANDLE_VALUE;

		if (!Found) 
		{
			TRACE(_T("\nThe specified device name was not found\n"));
			throw E_FAIL;
		}

		TRACE(_T("\nThe specified device name maps to:\n\n"));
		TRACE(_T("%s\n"), VolumeName);


	} catch(HRESULT hrex){
		hr = hrex;
	}

    if (FindHandle != INVALID_HANDLE_VALUE) 
    {
        FindVolumeClose(FindHandle);
        FindHandle = INVALID_HANDLE_VALUE;
    }

	return ((hr == S_OK) ? 1 : 0);
	*/
}

int SetupApiHelper::DetectDevicesSetupApi(CMapStringToPtr* devlist){
	GUID								hidGUID = GUID_DEVINTERFACE_VOLUME;
	//GUID								hidGUID = GUID_DEVINTERFACE_DISK;
	SP_DEVINFO_DATA						spDevInfoData;
	SP_INTERFACE_DEVICE_DATA			spIfaceDevData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA	pspDevDetail = NULL;
	HDEVINFO							hDevInfo;
	DEVINST								devInstParent;
    
	
	DWORD	nSize=0 ;
	TCHAR	buf[MAX_PATH];
	TCHAR	bufid[MAX_PATH];
	DWORD	needed;
	int		nret = 0;
	CString devname;
	CString devmanuf;
	CString logmsg;
	const int BUFFER_SIZE = 1024;
	BOOL bSelected = FALSE;


    hDevInfo = SetupDiGetClassDevs(&hidGUID, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
		ATLTRACE("Erreur SetUpDiGetClassDevs\n");
        return nret;
    }

	try {

		ZeroMemory(&spDevInfoData,sizeof(SP_DEVINFO_DATA));
		spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		for (int i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&spDevInfoData);i++) {
			ATLTRACE(_T("==========================\n"));
			if ( !SetupDiGetDeviceInstanceId(hDevInfo, &spDevInfoData, buf, sizeof(buf), &nSize) ) {
				continue;
			} 
			devname.Empty();
			ATLTRACE(_T("found device id = %s\n"), buf);

			ZeroMemory(&spIfaceDevData,sizeof(SP_INTERFACE_DEVICE_DATA));
			spIfaceDevData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
			if (!SetupDiEnumDeviceInterfaces(hDevInfo,NULL,&hidGUID,i,&spIfaceDevData)){
				throw E_FAIL;
			}
			//get needed size for iface detail
			SetupDiGetDeviceInterfaceDetail(hDevInfo,&spIfaceDevData,NULL,0,&needed,NULL);
			//on réserve la taille mémoire
			pspDevDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,needed);
			if (!pspDevDetail) {
				ATLTRACE("Memory allocation error on PSP_INTERFACE_DEVICE_DETAIL_DATA");
				throw E_FAIL;
			}

			pspDevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			ZeroMemory(&spDevInfoData, sizeof(spDevInfoData));
			spDevInfoData.cbSize = sizeof(spDevInfoData);
			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo,&spIfaceDevData,pspDevDetail,needed,NULL,&spDevInfoData)){
				ATLTRACE("No detail information : SetupDiGetDeviceInterfaceDetail");
				throw E_FAIL;
			}

			CString devpath = pspDevDetail->DevicePath;
			devpath.MakeLower();
			TRACE(_T("device path : ") + devpath + _T("\n"));
			logmsg = _T("device path : ") + devpath + _T("\n");
			//CLogger::GetInstance()->debug(logmsg, __WFILE_LINE__);

			//GetDeviceManuf(spDevInfoData.DevInst, devmanuf);

			GetDeviceName(spDevInfoData.DevInst, devname);
			ATLTRACE(_T("devname = %s\n"),devname);
			
			// Get the device instance of parent. This points to USBSTOR.
			CM_Get_Parent(&devInstParent,spDevInfoData.DevInst, 0);
			GetDeviceName(devInstParent, devname);
			ATLTRACE(_T("devname (USBSTOR : parent) = %s\n"),devname);
			CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
			ATLTRACE(_T("bufid (USBSTOR : parent) = %s\n"),bufid);
			
			CString devidFromUsbstor = bufid;
			CString devNameUsbstor = devname;


			// Get the device instance of parent. This points to USB.
			CM_Get_Parent(&devInstParent,devInstParent, 0);
			GetDeviceName(devInstParent, devname);
			ATLTRACE(_T("devname (USB : parent/parent) = %s\n"),devname);
			CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
			ATLTRACE(_T("bufid (USB : parent/parent)= %s\n"),bufid);

			CString devidFromUsb = bufid;
			CString devNameUsb = devname;

			CString strUsbBufId;
			CString savedDevName;
			if (hidGUID == GUID_DEVINTERFACE_DISK){
				strUsbBufId = devidFromUsbstor;
				savedDevName = devNameUsbstor;
			}
			if (hidGUID == GUID_DEVINTERFACE_VOLUME){
				strUsbBufId = devidFromUsb;
				savedDevName = devNameUsb;
			}
			strUsbBufId.MakeLower();


			long DeviceNumber = -1;

			//try to get a handle to the device to see if something in drive
			HANDLE hVolume = INVALID_HANDLE_VALUE;
			hVolume = CreateFile(devpath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
			if (hVolume != INVALID_HANDLE_VALUE) {
				
				STORAGE_DEVICE_NUMBER sdn;
				DWORD dwBytesReturned = 0;
				long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
				CloseHandle(hVolume);
				if ( res ) {
					DeviceNumber = sdn.DeviceNumber;
				}
			}
			TRACE(_T("device number %d\n"),DeviceNumber);
			if (DeviceNumber != -1){
				CString mountpath = GetMountPointForDevice(DeviceNumber);
				TRACE(_T("mountpath %s\n"),mountpath);

				/*
				if (devlist != NULL){

					CDeviceInfo* pInfo = NULL;
					devlist->Lookup(devpath,(void*&)pInfo);
					if (pInfo == NULL){
						pInfo = new CDeviceInfo();
					}
					pInfo->m_strCanonical = devpath;
					pInfo->m_strName = savedDevName;
					pInfo->m_strProductId = pid;
					pInfo->m_strStorageGB = storageGB;
					pInfo->m_strMacWifi = macWifi;
					pInfo->m_strMacEth = macEth;
					pInfo->m_strSerial = serial;
					pInfo->m_strMountPath = mountpath;
					pInfo->m_strUSBLayerId = strUsbBufId;
					pInfo->m_strImei = devInfo.m_strImei;
					//pInfo->m_strManufacturer = devmanuf;
					if (bInfoEx){
						pInfo->m_strFullPkey = fullPkey;
						pInfo->m_strDeviceVersion = versionInfo;
					}
					(*devlist)[devpath] = pInfo;
				}*/
			}

		}
	} catch(HRESULT hrex){
		if (!SUCCEEDED(hrex)){
			//CLogger::GetInstance()->error(_T("Error init device"), __WFILE_LINE__);
			nret = 0;
		}
	}

	//clean-up
	if (pspDevDetail) {
		HeapFree(GetProcessHeap(),0,pspDevDetail);
	}
	if (hDevInfo){
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	return nret;
}

CString SetupApiHelper::GetMountPointForDevice(DWORD deviceNumber){
	CString result;
	result.Empty();

	for (int i=0;i<32;i++){
		TCHAR drive = 'A' + i;
		TRACE("Check mount point %c\n",drive);
		CString driveLetter;
		driveLetter.Empty();
		driveLetter += drive;

		CString devpath;
		devpath.Format(_T("\\\\.\\%s:"),driveLetter);

		CString dosdevpath;
		dosdevpath.Format(_T("%s:"),driveLetter);

		HANDLE hVolume = INVALID_HANDLE_VALUE;
		hVolume = CreateFile(devpath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if (hVolume != INVALID_HANDLE_VALUE) {
			STORAGE_DEVICE_NUMBER sdn;
			DWORD dwBytesReturned = 0;
			long DeviceNumber = -1;
			long res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
			CloseHandle(hVolume);
			if ( res && (deviceNumber==sdn.DeviceNumber)) {
				result = dosdevpath;
			}
		}
					
	}

	return result;
}