/*
 *	\file devicemanager.cpp
 *	Java Usb Device Management [libjpmp.so]
 *	Copyright (C) 2008  cmi [netcmi78@gmail.com]
 *	Based on Media Transfer Protocol library libmtp
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *	
 */

#include "stdafx.h"
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h> 

#include <initguid.h>
//F18A0E88-C30C-11D0-8815-00A0C906BED8
DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB, 0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8);
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE , 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

#include <wmdmlog.h>
#include <wmdmlog_i.c>

#include "usbdevice.h"
#include "defines.h"
#include "devicemanager.h"
#include "isyncnotifier.h"
#include "usbdevmanapp.h"
#include "helperlib.h"


typedef struct _USB_HCD_DRIVERKEY_NAME {  
	ULONG  ActualLength;  
	WCHAR  DriverKeyName[1];
} USB_HCD_DRIVERKEY_NAME, *PUSB_HCD_DRIVERKEY_NAME;

typedef enum _USB_HUB_NODE {
	UsbHub,
	UsbMIParent
} USB_HUB_NODE;
typedef enum _USB_CONNECTION_STATUS {
	NoDeviceConnected,
	DeviceConnected,
	DeviceFailedEnumeration,
	DeviceGeneralFailure,
	DeviceCausedOvercurrent,
	DeviceNotEnoughPower,
	DeviceNotEnoughBandwidth,
	DeviceHubNestedTooDeeply,
	DeviceInLegacyHub
} USB_CONNECTION_STATUS, *PUSB_CONNECTION_STATUS;

typedef struct _tUSB_ENDPOINT_DESCRIPTOR { 
  UCHAR  bLength ;
  UCHAR  bDescriptorType ;
  UCHAR  bEndpointAddress ;
  UCHAR  bmAttributes ;
  USHORT  wMaxPacketSize ;
  UCHAR  bInterval ;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR ;

typedef struct _tUSB_PIPE_INFO {
	USB_ENDPOINT_DESCRIPTOR  EndpointDescriptor;
	ULONG  ScheduleOffset;
} USB_PIPE_INFO, *PUSB_PIPE_INFO;

#define USB_REQUEST_GET_DESCRIPTOR        0x06
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 0x02
#define USB_STRING_DESCRIPTOR_TYPE        0x03
#define FILE_DEVICE_USB FILE_DEVICE_UNKNOWN
#define USB_GET_NODE_INFORMATION 258
#define HCD_GET_ROOT_HUB_NAME 258
#define USB_GET_NODE_CONNECTION_INFORMATION 259
#define USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION 260
#define USB_GET_NODE_CONNECTION_NAME 261
#define HCD_GET_DRIVERKEY_NAME 265
#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION \
  CTL_CODE(FILE_DEVICE_USB, USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION \
  CTL_CODE(FILE_DEVICE_USB, USB_GET_NODE_CONNECTION_INFORMATION, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USB_GET_NODE_CONNECTION_NAME \
  CTL_CODE(FILE_DEVICE_USB, USB_GET_NODE_CONNECTION_NAME, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USB_GET_NODE_INFORMATION \
  CTL_CODE(FILE_DEVICE_USB, USB_GET_NODE_INFORMATION, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_HCD_DRIVERKEY_NAME \
  CTL_CODE(FILE_DEVICE_USB, HCD_GET_DRIVERKEY_NAME, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_USB_GET_ROOT_HUB_NAME \
  CTL_CODE(FILE_DEVICE_USB, HCD_GET_ROOT_HUB_NAME, METHOD_BUFFERED, FILE_ANY_ACCESS)


const char ClassName[] [20] = {
	"Reserved",			"Audio",				"Communications",		"Human Interface",
	"Monitor",			"Physical Interface",	"Power",				"Printer",
	"Storage",			"Hub",					"Vendor Specific",		"*ILLEGAL VALUE*"
	};

const char ConnectionStatus[] [30] = {
	"No device connected",			"Device connected",					"Device FAILED enumeration",
	"Device general FAILURE",		"Device caused overcurrent",		"Not enough power for device"
	};

// Define all stuctures using UCHAR or BOOLEAN so that the variables are not 'aligned' by the compiler
typedef struct _tDESCRIPTOR_REQUEST {
    ULONG ConnectionIndex;
    struct {
			UCHAR bmRequest; 
			UCHAR bRequest; 
			UCHAR wValue[2]; 
			UCHAR wIndex[2]; 
			UCHAR wLength[2];
		} SetupPacket;
    UCHAR Data[2048];
}DESCRIPTOR_REQUEST;	

typedef struct _tUSB_DEVICE_DESCRIPTOR {
    UCHAR bLength;					
	UCHAR bDescriptorType;				
	UCHAR bcdUSB[2];
    UCHAR bDeviceClass;				
	UCHAR bDeviceSubClass;				
	UCHAR bDeviceProtocol;
    UCHAR bMaxPacketSize0;			
	UCHAR idVendor[2];					
	UCHAR idProduct[2];
    UCHAR bcdDevice[2];				
	UCHAR iManufacturer;				
	UCHAR iProduct;
    UCHAR iSerialNumber;			
	UCHAR bNumConfigurations;
}USB_DEVICE_DESCRIPTOR;

typedef struct _tHUB_DESCRIPTOR {
    UCHAR bDescriptorLength;		
	UCHAR bDescriptorType;				
	UCHAR bNumberOfPorts;
	UCHAR wHubCharacteristics[2];	
	UCHAR bPowerOnToPowerGood;			
	UCHAR bHubControlCurrent;
    UCHAR bRemoveAndPowerMask[64];      
}HUB_DESCRIPTOR;

typedef struct _tNODE_INFORMATION {
    USB_HUB_NODE NodeType;			
	HUB_DESCRIPTOR HubDescriptor;		
	BOOLEAN HubIsBusPowered;
}NODE_INFORMATION; 

typedef struct _tNODE_CONNECTION_INFORMATION {
    ULONG ConnectionIndex;			
	USB_DEVICE_DESCRIPTOR DeviceDescriptor;	
	UCHAR CurrentConfigurationValue;
    BOOLEAN LowSpeed;				
	BOOLEAN DeviceIsHub;				
	UCHAR DeviceAddress[2];
    UCHAR NumberOfOpenPipes[4];		
	UCHAR ConnectionStatus[4];			
	USB_PIPE_INFO PipeList[32];
}NODE_CONNECTION_INFORMATION;

static BYTE abPVK[] = {
        0x00
};
static BYTE abCert[] = {
        0x00
};

devicemanager::devicemanager(){
	InitializeCriticalSection(&_criticalSection);

	m_spWMDMLogger = NULL;

	HRESULT hr = CoCreateInstance(
		CLSID_WMDMLogger,
		NULL,
		CLSCTX_ALL,
		IID_IWMDMLogger,
		(void**)&m_spWMDMLogger
		);

	if ( (hr == S_OK) && (m_spWMDMLogger.p != NULL) ){
		m_spWMDMLogger->Enable(FALSE);
	}

	m_pWMDevMgr = NULL;
	m_pICallbackObject = NULL;
	m_dwNotificationCookie = -1;
	m_pSAC = NULL;
	setup_mtp();

	
}

devicemanager::~devicemanager(){
	clear_devices();
	unset_mtp();
	SAFE_COMPTR_RELEASE(m_spWMDMLogger);
	DeleteCriticalSection(&_criticalSection);
}

void devicemanager::setup_mtp(){
	HRESULT hr = S_OK;
    CComPtr<IComponentAuthenticate> pAuth = NULL;

	try{
    
		// Initialize member variables
		//
		m_pSAC        = NULL;
		m_pWMDevMgr   = NULL;

		hr = CoCreateInstance(
			CLSID_MediaDevMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IComponentAuthenticate,
			(void**)&pAuth
		);
		SUCCEEDED(hr) ? 0 : throw hr;

		// Create the client authentication object
		//
		m_pSAC = new CSecureChannelClient;
		if (m_pSAC ==NULL) throw E_FAIL;

		// Select the cert and the associated private key into the SAC
		//
		hr = m_pSAC->SetCertificate(
			SAC_CERT_V1,
			(BYTE *)abCert, sizeof(abCert),
			(BYTE *)abPVK,  sizeof(abPVK)
		);
		SUCCEEDED(hr) ? 0 : throw hr;
	            
		// Select the authentication interface into the SAC
		//
		m_pSAC->SetInterface( pAuth );

		// Authenticate with the V1 protocol
		//
		hr = m_pSAC->Authenticate( SAC_PROTOCOL_V1 );
		SUCCEEDED(hr) ? 0 : throw hr;

		// Authenticated succeeded, so we can use the WMDM functionality.
		// Acquire an interface to the top-level WMDM interface.
		//
		hr = pAuth->QueryInterface( IID_IWMDeviceManager, (void**)&m_pWMDevMgr );
		SUCCEEDED(hr) ? 0 : throw hr;

		hr = S_OK;
	}catch(HRESULT hrex){
		hr = hrex;
	}

	SAFE_COMPTR_RELEASE(pAuth);
    m_hrInit = hr;
}

void devicemanager::unset_mtp(){

    // Release the top-level WMDM interface
    //
	SAFE_RELEASE(m_pWMDevMgr);
	
    // Release the SAC
    //
	if( m_pSAC ){
		delete m_pSAC;
		m_pSAC = NULL;
	}

	SAFE_RELEASE(m_pICallbackObject);
}

int devicemanager::detect_devices_msc(){
	
	int retcode = 0;

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
	int		nret = 1;
	std::wstring devname;
	std::wstring devmanuf;
	std::wstring logmsg;
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
			devname.clear();
			ATLTRACE(_T("found device id = %s\n"), buf);
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"found volume id " << buf << std::endl;
			}

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

			std::wstring devpath = pspDevDetail->DevicePath;
			std::transform(devpath.begin(), devpath.end(), devpath.begin(), tolower);

			ATLTRACE(L"device path : %s\n" , devpath);
			logmsg = L"device path : " + devpath + L"\n";
			//CLogger::GetInstance()->debug(logmsg, __WFILE_LINE__);
			if (devpath.find_first_of(L"removablemedia") == std::string::npos){
				ATLTRACE("Not removeable media\n");
				continue;
			}


			GetDeviceName(spDevInfoData.DevInst, devname);
			ATLTRACE(_T("devname = %s\n"),devname);
			
			// Get the device instance of parent. This points to USBSTOR.
			CM_Get_Parent(&devInstParent,spDevInfoData.DevInst, 0);
			GetDeviceName(devInstParent, devname);
			ATLTRACE(_T("devname (USBSTOR : parent) = %s\n"),devname);
			CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
			ATLTRACE(_T("bufid (USBSTOR : parent) = %s\n"),bufid);
			
			std::wstring devidFromUsbstor = bufid;
			std::wstring devNameUsbstor = devname;


			// Get the device instance of parent. This points to USB.
			CM_Get_Parent(&devInstParent,devInstParent, 0);
			GetDeviceName(devInstParent, devname);
			ATLTRACE(_T("devname (USB : parent/parent) = %s\n"),devname);
			CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
			ATLTRACE(_T("bufid (USB : parent/parent)= %s\n"),bufid);


			//GetDeviceManuf(devInstParent, devmanuf);

			std::wstring devidFromUsb = bufid;
			std::wstring devNameUsb = devname;

			std::wstring rootHubName;
			std::wstring rootHubId;
			CM_Get_Parent(&devInstParent,devInstParent, 0);
			GetDeviceName(devInstParent, devname);
			ATLTRACE(_T("devname (ROOT HUB : parent/parent) = %s\n"),devname);
			CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
			ATLTRACE(_T("bufid (ROOT HUB : parent/parent)= %s\n"),bufid);
			rootHubName = devname;
			rootHubId = bufid;

			std::wstring strUsbBufId;
			std::wstring savedDevName;
			if (hidGUID == GUID_DEVINTERFACE_DISK){
				strUsbBufId = devidFromUsbstor;
				savedDevName = devNameUsbstor;
			}
			if (hidGUID == GUID_DEVINTERFACE_VOLUME){
				strUsbBufId = devidFromUsb;
				savedDevName = devNameUsb;
			}

			std::transform(strUsbBufId.begin(), strUsbBufId.end(), strUsbBufId.begin(), tolower);

			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"found volume id " << strUsbBufId << std::endl;
				std::wcout << L"found volume name " << savedDevName << std::endl;
			}

			usbdevice usbdev;
			usbdev.set_canonical(devpath);

			long DeviceNumber = -1;

			//try to get a handle to the device to see if something in drive
			HANDLE hVolume = INVALID_HANDLE_VALUE;
			hVolume = CreateFile(
				devpath.c_str(), 
				0, 
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_EXISTING, 
				NULL, 
				NULL);

			if (hVolume != INVALID_HANDLE_VALUE) {
				
				// need storage device number to get mount point
				STORAGE_DEVICE_NUMBER sdn;
				DWORD dwBytesReturned = 0;
				BOOL res = DeviceIoControl(hVolume, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
				
				if ( !res ) {
					CloseHandle(hVolume);
					continue;
				}

				// copy device number for further utilisation
				DeviceNumber = sdn.DeviceNumber;

				// get USB descriptor properties from USB hub
				// first get USB HUB
				std::wstring rootHubPath = rootHubId;
				GUID guidusb = GUID_DEVINTERFACE_USB_HUB;
				CString temp = rootHubPath.c_str();
				temp.Replace(L"\\",L"#");
				rootHubPath = temp;
				rootHubPath += L"#";

				WCHAR szguid[MAX_PATH+1];
				int sz = MAX_PATH;
				StringFromGUID2(guidusb,szguid,sz);
				rootHubPath += szguid;

				
				rootHubPath = L"\\\\.\\" + rootHubPath;
				res = get_descriptor_props(rootHubPath,strUsbBufId,&usbdev);

				

				CloseHandle(hVolume);

				if (res != 0) {
					continue;
				}

			}
			ATLTRACE(_T("device number %d\n"),DeviceNumber);
			if (DeviceNumber != -1){
				CString mountpath = GetMountPointForDevice(DeviceNumber);
				ATLTRACE(_T("mountpath %s\n"),mountpath);
				
				// try to get volume info to check realy mounted
				WCHAR lpVolumeNameBuffer[MAX_PATH + 1];
				WCHAR lpFileSystemNameBuffer[MAX_PATH + 1];
				DWORD nVolumeNameSize = MAX_PATH;
				DWORD dwVolumeSerialNumber;
				DWORD dwMaximumComponentLength;
				DWORD dwFileSystemFlags;
				DWORD nFileSystemNameSize = MAX_PATH;

				BOOL b = GetVolumeInformation(
						mountpath,
						lpVolumeNameBuffer,
						nVolumeNameSize,
						&dwVolumeSerialNumber,
						&dwMaximumComponentLength,
						&dwFileSystemFlags,
						lpFileSystemNameBuffer,
						nFileSystemNameSize);
				if ( !b  ) {
					DWORD dwerr = GetLastError();
					if (usbdevman_app::get_instance()->is_verbose() ){
						std::wcerr << L" *** ERROR *** Error retrieving volume information, error code = " << dwerr << std::endl;
					}
					continue;
				}
					
				usbdev.set_devicemode(usbdevice::MODE_MSC);
				usbdev.set_mountpoint(std::wstring(mountpath));
				usbdev.dump();

				ATLTRACE(_T("to be kept !!!\n"));
				ATLTRACE(_T("- - - - - - - - -\n"));
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcout << L" this device is accepted" << std::endl;
				}
				EnterCriticalSection(&_criticalSection); 
				listDevices[usbdev.get_canonical()] = new usbdevice(usbdev);
				LeaveCriticalSection(&_criticalSection); 

			}

		}
	} catch(HRESULT hrex){
		if (!SUCCEEDED(hrex)){
			//CLogger::GetInstance()->error(_T("Error init device"), __WFILE_LINE__);
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcerr << L" *** ERROR *** Exception detecting MSC devices, error code = " << hrex << std::endl;
			}
			retcode = 1;
		}
	}

	//clean-up
	if (pspDevDetail) {
		HeapFree(GetProcessHeap(),0,pspDevDetail);
	}
	if (hDevInfo){
		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	//std::cout << "exit detect_devices with code " << retcode << std::endl;
	return retcode;
}


int devicemanager::refresh_devices(){
	int ret = 0;
	ret = clear_devices();
	if (ret != 0) {
		if (usbdevman_app::get_instance()->is_verbose() ){
			printf("clear_devices nok\n");
		}
		return ret;
	}
	if (usbdevman_app::get_instance()->is_verbose() ){
		printf("clear_devices ok\n");
	}
	ret = detect_devices_mtp();
	if (ret != 0) {
		printf("detect_devices mtp nok\n");
	}
	ret = detect_devices_msc();
	if (ret != 0) {
		printf("detect_devices msc nok\n");
	}
	return ret;
}

/* check if device in list */
int devicemanager::has_device(std::wstring devid){
	usbdevice* usbdev = (usbdevice*)listDevices[devid];
	if (usbdev == NULL){
		return 0;
	} else {
		return 1;
	}
}

/* check if device still connected */
int devicemanager::check_device(std::wstring devid){
	/*clear_devices();
	detect_devices_msc();
	usbdevice* asdev = (usbdevice*)listDevices[devid];
	if (asdev){
		return 0;
	} else {
		return 1;
	}*/

	int ret = 1;

	// check only for 10 host controllers

	for (int i=0; i< 10; i++){
		CString hostPath;
		hostPath.Format(L"\\\\.\\HCD%d",  i);
		std::wstring hostControllerPath = std::wstring(hostPath);

		HANDLE hControllerHandle = CreateFile(
			hostControllerPath.c_str(), 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL);

		if (hControllerHandle == INVALID_HANDLE_VALUE){
			continue;
		}
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcout << L"Host Controller " << hostControllerPath.c_str() << L" found. " << std::endl;
		}

		// loop on each host controller

		DWORD dwBytesReturned;
		USB_HCD_DRIVERKEY_NAME usbhcd;
		ZeroMemory(&usbhcd,sizeof(USB_HCD_DRIVERKEY_NAME));

		BOOL result = DeviceIoControl(
			hControllerHandle, 
			IOCTL_USB_GET_ROOT_HUB_NAME, 
			NULL, 
			0, 
			&usbhcd, 
			sizeof(usbhcd), 
			&dwBytesReturned, 
			NULL);

		DWORD dwBytesNeeded = usbhcd.ActualLength;
		USB_HCD_DRIVERKEY_NAME* pusbhcd = (USB_HCD_DRIVERKEY_NAME*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		result = DeviceIoControl(
			hControllerHandle, 
			IOCTL_USB_GET_ROOT_HUB_NAME, 
			pusbhcd, 
			dwBytesNeeded, 
			pusbhcd, 
			dwBytesNeeded, 
			&dwBytesReturned, 
			NULL);

		std::wstring foundRootHubName = L"\\\\.\\";
		foundRootHubName += (LPWSTR)&(pusbhcd->DriverKeyName[0]);

		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pusbhcd);

		if (!result){
			CloseHandle(hControllerHandle);
			hControllerHandle = INVALID_HANDLE_VALUE;
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << "IOCTL_USB_GET_ROOT_HUB_NAME returned error " << GetLastError() << std::endl;
			}
			continue;
		}


		NODE_INFORMATION NodeInformation;
		NODE_CONNECTION_INFORMATION ConnectionInformation;
		int portCount = 0;
		HANDLE hHubHandle = INVALID_HANDLE_VALUE;

		// get hub handle

		try{
		
			hHubHandle = CreateFile(
				foundRootHubName.c_str(), 
				GENERIC_WRITE, 
				FILE_SHARE_WRITE , 
				NULL, 
				OPEN_EXISTING, 
				NULL, 
				NULL);

			if (hHubHandle == INVALID_HANDLE_VALUE) {
				throw GetLastError();
			}

			// get the Hub Information
			ret = DeviceIoControl(
				hHubHandle, 
				IOCTL_USB_GET_NODE_INFORMATION, 
				&NodeInformation,
				sizeof(NodeInformation), 
				&NodeInformation, 
				sizeof(NodeInformation), 
				&dwBytesReturned, 
				NULL);
			if (!ret) {
				throw GetLastError();
			}

			portCount = NodeInformation.HubDescriptor.bNumberOfPorts;

			for (int UsbPortIndex=1; UsbPortIndex< portCount + 1; UsbPortIndex++){
				int LanguageID = 0;  // Reset for each port
				ConnectionInformation.ConnectionIndex = UsbPortIndex;
				ret = DeviceIoControl(
					hHubHandle, 
					IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, 
					&ConnectionInformation,
					sizeof(ConnectionInformation), 
					&ConnectionInformation, 
					sizeof(ConnectionInformation), 
					&dwBytesReturned, 
					NULL);
				if (!ret) {
					if (usbdevman_app::get_instance()->is_verbose() ){
						std::wcerr << L" *** ERROR *** Error retrieving node connection information" << std::endl;
					}
					throw GetLastError();
				}

				if (!ConnectionInformation.DeviceIsHub) {
					std::wstring manuf;
					LanguageID  =get_stringdescriptor(
						hHubHandle,
						UsbPortIndex,
						LanguageID,
						ConnectionInformation.DeviceDescriptor.iManufacturer,
						manuf);

					std::wstring serial;
					LanguageID  =get_stringdescriptor(
						hHubHandle,
						UsbPortIndex,
						LanguageID,
						ConnectionInformation.DeviceDescriptor.iSerialNumber,
						serial);

					std::wstring name;
					LanguageID  =get_stringdescriptor(
						hHubHandle,
						UsbPortIndex,
						LanguageID,
						ConnectionInformation.DeviceDescriptor.iProduct,
						name);

					unsigned int vid = MAKEWORD( 
						ConnectionInformation.DeviceDescriptor.idVendor[0],
						ConnectionInformation.DeviceDescriptor.idVendor[1]);
					unsigned int pid = MAKEWORD( 
						ConnectionInformation.DeviceDescriptor.idProduct[0],
						ConnectionInformation.DeviceDescriptor.idProduct[1]);

					CString usbid;
					//a504_CYGKU96FWK49_0e79_1306
					usbid.Format(L"%s_%s_%04X_%04X", name.c_str(), serial.c_str(), vid, pid );
					usbid.MakeLower();

					std::wstring inputid = devid;
					std::transform(inputid.begin(), inputid.end(), inputid.begin(), tolower);

					if (inputid == std::wstring(usbid)) {
						// device found
						ret = 0;
						throw (DWORD)0;
					}
				} else {
					//FIXME loop on sub hubs!!!
				}
			}
		} catch(DWORD lasterr){
			if (lasterr != 0){
				if (usbdevman_app::get_instance()->is_verbose() ){
					std::wcerr << L" *** ERROR *** Exception Error " << lasterr << std::endl;
				}
				ret = lasterr;
			}
		}
		if (hHubHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(hHubHandle);
			hHubHandle = INVALID_HANDLE_VALUE;
		}
		if (hHubHandle != INVALID_HANDLE_VALUE) {
			CloseHandle(hControllerHandle);
			hControllerHandle = INVALID_HANDLE_VALUE;
		}
		if (ret == 0){
			break;
		}
	}
	
	return ret;

}

/* release the device from the list */
int devicemanager::release_device(std::wstring devid){
	int ret = 0;
	EnterCriticalSection(&_criticalSection);
	
	usbdevice* usbdev = listDevices[devid];
	if (usbdev){
		delete(usbdev);
		usbdev = NULL;
		listDevices.erase(devid);
	}
	
	LeaveCriticalSection(&_criticalSection);
	return ret;
}

usbdevice* devicemanager::get_devicebyid(std::wstring devid){
	usbdevice* usbdev = NULL; 
	int ret = 0;

	EnterCriticalSection(&_criticalSection);
	usbdev = listDevices[devid];
	LeaveCriticalSection(&_criticalSection);
	
	return usbdev;
}

int devicemanager::fill_devicesinfo(UsbDevicesInfo* list){
	int ret = 0;

	EnterCriticalSection(&_criticalSection);

	for(devicemanager::UsbDevices::iterator iter = listDevices.begin(); iter != listDevices.end(); ++iter) {
		usbdevice* usbdev = (*iter).second;
		if (usbdev){
			usbdevice_info usbinfo;
			usbinfo.set_canonical(usbdev->get_canonical());
			usbinfo.set_name(usbdev->get_name());
			usbinfo.set_serial(usbdev->get_serial());
			usbinfo.set_pid(usbdev->get_pid());
			usbinfo.set_vid(usbdev->get_vid());
			usbinfo.set_productid(usbdev->get_productid());
			usbinfo.set_vendorid(usbdev->get_vendorid());
			usbinfo.set_manufacturer(usbdev->get_manufacturer());
			usbinfo.set_mountpoint(usbdev->get_mountpoint());
			usbinfo.set_videodir(usbdev->get_videodir());
			usbinfo.set_audiodir(usbdev->get_audiodir());
			usbinfo.set_pictdir(usbdev->get_pictdir());
			usbinfo.set_idx(usbdev->get_idx());			
			usbinfo.set_freesize(usbdev->get_freesize());
			usbinfo.set_totalsize(usbdev->get_totalsize());
			usbinfo.set_devicemode(usbdev->get_devicemode());
			(*list)[usbinfo.get_canonical()] = usbinfo;
		}
	}
	LeaveCriticalSection(&_criticalSection);
	return ret;
}


int devicemanager::clear_devices(){
	
	int ret = 0;

	EnterCriticalSection(&_criticalSection); 
	for(devicemanager::UsbDevices::iterator iter = listDevices.begin(); iter != listDevices.end(); ++iter) {
		usbdevice* usbdev = (*iter).second;
		if (usbdev){
			delete(usbdev);
			usbdev = NULL;
		}
	}
	listDevices.clear();

	LeaveCriticalSection(&_criticalSection); 
	
	return ret;
}

int devicemanager::dump_devices(){
	int ret = 0;
	if (ret != 0 ) return ret;

	EnterCriticalSection(&_criticalSection); 
	for(devicemanager::UsbDevices::iterator iter = listDevices.begin(); iter != listDevices.end(); ++iter) {
		usbdevice* usbdev = (*iter).second;
		if (usbdev){
			std::wcout << L"entry for key : " << (*iter).first << std::endl;
			usbdev->dump();
		}
	}
	LeaveCriticalSection(&_criticalSection); 
	return ret;
}




int devicemanager::detect_devices_mtp(){
	int retcode = 0;

	HRESULT hr = S_OK;
	CComPtr<IWMDeviceManager2> spIWmdm2;
	CComPtr<IWMDMEnumDevice> pIEnmDvc;
	BOOL bDeviceDetected = FALSE;

	if (usbdevman_app::get_instance()->is_verbose() ){
		std::wcout << L"Check MTP devices " << std::endl;
	}
	try{
		hr = m_pWMDevMgr->QueryInterface(&spIWmdm2);
		if (hr==S_OK && spIWmdm2.p){

			DWORD dwDevices;
			HRESULT hr = spIWmdm2->GetDeviceCount(&dwDevices);
			if (!SUCCEEDED(hr)){
				CString msg;
				msg.Format(TEXT("Error GetDeviceCount %d"),hr);
				throw hr;
			}
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcout << L"The WMDM device manager reports " << dwDevices << L" compatible devices " << std::endl;
			}
			if (dwDevices>0){
				hr = spIWmdm2->EnumDevices2(&pIEnmDvc);
				if (!SUCCEEDED(hr)){
					CString msg;
					msg.Format(TEXT("Error EnumDevices2 %d"),hr);
					throw hr;
				}
				for (unsigned int i=0;i<dwDevices;i++){
					IWMDMDevice* pIdv = NULL;
					ULONG ulOut = 0;
					// Enumerate the attached devices. Take the first one.
					hr = pIEnmDvc->Next(1, &pIdv, &ulOut);
					if (!SUCCEEDED(hr)){
						CString msg;
						msg.Format(TEXT("Error Next  %d"),hr);
						throw hr;
					}

					CString canonical = GetMtpDeviceCanonical(pIdv);
					if (usbdevman_app::get_instance()->is_verbose() ){
						ATLTRACE(L"%s\n",canonical);
						std::wcout << L"canonical = " << std::wstring(canonical) << std::endl;
					}
					canonical.MakeLower();

					// check if USB root
					if (canonical.Left(7) != L"\\\\?\\usb"){
						SAFE_RELEASE(pIdv);
						continue;
					}

					// format the deviceID string like USB#PID_xxx&VID_xxx#ssssssssssss
					// from what we get as canonical
					CString deviceIDfromCanonical = canonical;
					int idx = deviceIDfromCanonical.ReverseFind(L'#');
					if (idx != -1){
						deviceIDfromCanonical = deviceIDfromCanonical.Left(idx);
					}
					idx = deviceIDfromCanonical.Find(L"usb");
					if (idx != -1){
						deviceIDfromCanonical = deviceIDfromCanonical.Mid(idx);
					}
					deviceIDfromCanonical.Replace(L"#",L"\\");

					// get the device index if any
					// look for last of '$'
					idx = canonical.ReverseFind(L'$');
					UINT deviceindex = 0;
					if (idx != -1){
						deviceindex = _wtoi(canonical.Mid(idx + 1));
					}


					//----------- 
					//  use SetupDiGetClassDevs to retrieve the device IDs 
					//  and compare to deviceID retrieved before
					// 	

					GUID								hidGUID = GUID_DEVINTERFACE_USB_DEVICE;
					SP_DEVINFO_DATA						spDevInfoData;
					PSP_DEVICE_INTERFACE_DETAIL_DATA	pspDevDetail = NULL;
					HDEVINFO							hDevInfo;
					DEVINST								devInstParent;
					
					DWORD	nSize=0 ;
					TCHAR	buf[MAX_PATH];
					TCHAR	bufid[MAX_PATH];
					int		nret = 1;
					std::wstring devname;
					std::wstring devmanuf;
					std::wstring logmsg;
					const int BUFFER_SIZE = 1024;
					BOOL bSelected = FALSE;


					hDevInfo = SetupDiGetClassDevs(&hidGUID, 0, 0, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

					if (hDevInfo == INVALID_HANDLE_VALUE) {
						ATLTRACE("Erreur SetUpDiGetClassDevs\n");
						return nret;
					}

					std::wstring devidFromUsb = L"";
					std::wstring rootHubName= L"";
					std::wstring rootHubId= L"";
					BOOL bFound = FALSE;

					try {

						ZeroMemory(&spDevInfoData,sizeof(SP_DEVINFO_DATA));
						spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
						for (int i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&spDevInfoData);i++) {
							ATLTRACE(_T("==========================\n"));
							if ( !SetupDiGetDeviceInstanceId(hDevInfo, &spDevInfoData, buf, sizeof(buf), &nSize) ) {
								continue;
							} 
							devname.clear();
							ATLTRACE(_T("found device id = %s\n"), buf);
							if (usbdevman_app::get_instance()->is_verbose() ){
								std::wcout << L"found device id " << buf << std::endl;
							}

							devidFromUsb = buf;
							std::transform(devidFromUsb.begin(), devidFromUsb.end(), devidFromUsb.begin(), tolower);

							if (std::wstring(deviceIDfromCanonical) != devidFromUsb){
								// not the deviceID we are looking for, skip
								continue;
							}

							// found same deviceID, get the USB HUB root
							CM_Get_Parent(&devInstParent,spDevInfoData.DevInst, 0);
							GetDeviceName(devInstParent, devname);
							ATLTRACE(_T("devname (ROOT HUB : parent/parent) = %s\n"),devname);
							CM_Get_Device_ID(devInstParent, bufid, BUFFER_SIZE,0);
							ATLTRACE(_T("bufid (ROOT HUB : parent/parent)= %s\n"),bufid);
							rootHubName = devname;
							rootHubId = bufid;
							bFound = TRUE;
							break;

						}
					} catch(HRESULT hrex){
						hr = hrex;
					}
					// some cleanup
					if (pspDevDetail) {
						HeapFree(GetProcessHeap(),0,pspDevDetail);
						pspDevDetail = NULL;
					}
					if (hDevInfo){
						SetupDiDestroyDeviceInfoList(hDevInfo);
						hDevInfo = NULL;
					}

					// check if something found
					if (!bFound) continue;


					// OK, here we got the USB HUB root for this device
					// and the deviceID, we can create a new instance
					usbdevice *pusbdev = new usbdevice();
					pusbdev->set_idx(deviceindex);
					if( pusbdev ) {
						// init from WMDM device interface
						hr = pusbdev->init_mtp( pIdv );
						if (hr != S_OK || pusbdev->get_devicemode() != usbdevice::MODE_MTP){
							// something wrong or MSC mode, skip
							delete(pusbdev);
							pusbdev = NULL;
						} else {

							// need to get USB descriptio information
							// we format the USB root path - need to replace the '\' with '#'
							// and to add the GUID_DEVINTERFACE_USB_HUB
							std::wstring rootHubPath = rootHubId;
							GUID guidusb = GUID_DEVINTERFACE_USB_HUB;
							CString temp = rootHubPath.c_str();
							temp.Replace(L"\\",L"#");
							rootHubPath = temp;
							rootHubPath += L"#";

							WCHAR szguid[MAX_PATH+1];
							int sz = MAX_PATH;
							StringFromGUID2(guidusb,szguid,sz);
							rootHubPath += szguid;

							rootHubPath = L"\\\\.\\" + rootHubPath;

							// we finally got the USB root path and the deviceID
							// we can get USB description infos
							BOOL res = get_descriptor_props(rootHubPath,devidFromUsb,pusbdev);

							// put it in list
							EnterCriticalSection(&_criticalSection); 
							listDevices[pusbdev->get_canonical()] = pusbdev;
							LeaveCriticalSection(&_criticalSection); 
						}
					}
					//-----------
					
				}
			}
			throw S_OK;
		}
	}
	catch(HRESULT hrex){
		hr = hrex;
		if (hr!=S_OK){
			CString sMsg;
			sMsg.Format(_T("0x%X"),hrex);
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcerr << L" *** ERROR *** Exception detecting MTP devices, error code = " << sMsg << std::endl;
			}
			retcode = 1;
		}
	}
	SAFE_COMPTR_RELEASE(spIWmdm2);
	SAFE_COMPTR_RELEASE(pIEnmDvc);

	return retcode;
}

CString devicemanager::GetMountPointForDevice(DWORD deviceNumber){
	CString result;
	result.Empty();

	for (int i=0;i<32;i++){
		TCHAR drive = L'A' + i;
		//ATLTRACE("Check mount point %c\n",drive);
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

CONFIGRET devicemanager::GetDeviceName(DEVNODE DevNode, std::wstring& devName){
	CONFIGRET cr = 0;
	CString	strType;
    CString	strValue;
    LPTSTR	lpszBuffer;

	devName.clear();
    
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

int devicemanager::get_descriptor_props(std::wstring rootHubPath, std::wstring deviceUsbId, usbdevice* usbdev){
	int ret = 0;

	DWORD dwBytesReturned;
	NODE_INFORMATION NodeInformation;
	NODE_CONNECTION_INFORMATION ConnectionInformation;
	int portCount = 0;

	// get hub handle
	HANDLE hHubHandle = INVALID_HANDLE_VALUE;
	hHubHandle = CreateFile(
		rootHubPath.c_str(), 
		GENERIC_WRITE, 
		FILE_SHARE_WRITE , 
		NULL, 
		OPEN_EXISTING, 
		NULL, 
		NULL);

	if (hHubHandle == INVALID_HANDLE_VALUE) {
		return GetLastError();
	}

	// get the Hub Information
	ret = DeviceIoControl(
		hHubHandle, 
		IOCTL_USB_GET_NODE_INFORMATION, 
		&NodeInformation,
		sizeof(NodeInformation), 
		&NodeInformation, 
		sizeof(NodeInformation), 
		&dwBytesReturned, 
		NULL);
	if (!ret) {
		return GetLastError();
	}

	portCount = NodeInformation.HubDescriptor.bNumberOfPorts;

	for (int UsbPortIndex=1; UsbPortIndex< portCount + 1; UsbPortIndex++){
		int LanguageID = 0;  // Reset for each port
		ConnectionInformation.ConnectionIndex = UsbPortIndex;
		ret = DeviceIoControl(
			hHubHandle, 
			IOCTL_USB_GET_NODE_CONNECTION_INFORMATION, 
			&ConnectionInformation,
			sizeof(ConnectionInformation), 
			&ConnectionInformation, 
			sizeof(ConnectionInformation), 
			&dwBytesReturned, 
			NULL);
		if (!ret) {
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcerr << L" *** ERROR *** Error retrieving node connection information" << std::endl;
			}
			return GetLastError();
		}

		if (!ConnectionInformation.DeviceIsHub) {
			std::wstring manuf;
			LanguageID  =get_stringdescriptor(
				hHubHandle,
				UsbPortIndex,
				LanguageID,
				ConnectionInformation.DeviceDescriptor.iManufacturer,
				manuf);

			std::wstring serial;
			LanguageID  =get_stringdescriptor(
				hHubHandle,
				UsbPortIndex,
				LanguageID,
				ConnectionInformation.DeviceDescriptor.iSerialNumber,
				serial);

			std::wstring name;
			LanguageID  =get_stringdescriptor(
				hHubHandle,
				UsbPortIndex,
				LanguageID,
				ConnectionInformation.DeviceDescriptor.iProduct,
				name);

			unsigned int vid = MAKEWORD( 
				ConnectionInformation.DeviceDescriptor.idVendor[0],
				ConnectionInformation.DeviceDescriptor.idVendor[1]);
			unsigned int pid = MAKEWORD( 
				ConnectionInformation.DeviceDescriptor.idProduct[0],
				ConnectionInformation.DeviceDescriptor.idProduct[1]);

			CString usbid;
			usbid.Format(L"USB\\VID_%04X&PID_%04X\\%s", vid, pid, serial.c_str());
			usbid.MakeLower();
			if (deviceUsbId == std::wstring(usbid)){
				// device found
				usbdev->set_vendorid(vid);
				usbdev->set_productid(pid);

				CString temp;
				temp.Format(L"0x%04x", vid);
				usbdev->set_vid(std::wstring(temp));

				temp.Format(L"0x%04x", pid);
				usbdev->set_pid(std::wstring(temp));

				usbdev->set_manufacturer(std::wstring(manuf));
				usbdev->set_serial(std::wstring(serial));
				usbdev->set_name(std::wstring(name));
				ret = 0;
				break;
			}
			
			//LanguageID = DisplayDeviceDescriptor(hHubHandle, UsbPortIndex, LanguageID, &ConnectionInformation.DeviceDescriptor.bLength);
			//LanguageID = DisplayConfigurationDescriptor(hHubHandle, UsbPortIndex, LanguageID);
		} else {
			//FIXME loop on sub hubs!!!
		}
	
	}

	CloseHandle(hHubHandle);
	return ret;
}

USHORT devicemanager::get_stringdescriptor(HANDLE hHubHandle, ULONG UsbPortIndex, USHORT LanguageID, UCHAR descriptorIndex, std::wstring& descriptorValue){

	DESCRIPTOR_REQUEST request;
	DWORD dwBytesReturned;
	BOOL result;
	if (LanguageID == 0) {			
		// Get the language ID
		ZeroMemory(&request, sizeof(request));
		request.ConnectionIndex = UsbPortIndex;
		request.SetupPacket.bmRequest = 0x80;
		request.SetupPacket.bRequest = USB_REQUEST_GET_DESCRIPTOR;
		request.SetupPacket.wValue[1] = USB_STRING_DESCRIPTOR_TYPE;
		request.SetupPacket.wLength[0] = 4;
		result = DeviceIoControl(
			hHubHandle, 
			IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, 
			&request,
			sizeof(request), 
			&request, 
			sizeof(request), 
			&dwBytesReturned, 
			NULL);

		if (!result) {
			if (usbdevman_app::get_instance()->is_verbose() ){
				std::wcerr << L" *** ERROR *** Error retrieving string descriptor 0 , error code = "<< GetLastError() << std::endl;
			}
		}
		LanguageID = request.Data[2] + (request.Data[3] << 8);
	}

	ZeroMemory(&request, sizeof(request));
	request.ConnectionIndex = UsbPortIndex;
	request.SetupPacket.bmRequest = 0x80;
	request.SetupPacket.bRequest = USB_REQUEST_GET_DESCRIPTOR;
	request.SetupPacket.wValue[1] = USB_STRING_DESCRIPTOR_TYPE;
	request.SetupPacket.wValue[0] = descriptorIndex;
	request.SetupPacket.wIndex[0] = LanguageID & 0xFF;
	request.SetupPacket.wIndex[1] = (LanguageID >> 8) & 0xFF;
	request.SetupPacket.wLength[0] = 255;
	result = DeviceIoControl(
		hHubHandle, 
		IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION, 
		&request,
		sizeof(request), 
		&request, 
		sizeof(request), 
		&dwBytesReturned, 
		NULL);
	if (!result) {
		if (usbdevman_app::get_instance()->is_verbose() ){
			std::wcerr << L" *** ERROR *** Error retrieving string descriptor  " << descriptorIndex << L", error code = "<< GetLastError() << std::endl;
		}
	}
	descriptorValue = (LPWSTR)(&request.Data[2]);
	return LanguageID;
}

CString devicemanager::GetMtpDeviceCanonical(IWMDMDevice* pdevice){
	WCHAR wszCanonical[MAX_PATH];

	if (!pdevice) return L"";

	//Get canonical name
	IWMDMDevice2* pTempDevice2;
	HRESULT hr = pdevice->QueryInterface(IID_IWMDMDevice2, (void**)&pTempDevice2);
	if( FAILED(hr) ) {
		wcscpy_s( wszCanonical, L"");
	} else{
		hr = pTempDevice2->GetCanonicalName( wszCanonical, sizeof(wszCanonical)/sizeof(wszCanonical[0]) - 1 );
		if( FAILED(hr) ){
			 wcscpy_s( wszCanonical, L"");
		}
		SAFE_RELEASE(pTempDevice2);
	}
	return CString(wszCanonical);
}

