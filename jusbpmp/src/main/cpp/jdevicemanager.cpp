/*
 *	\file jdevicemanager.h
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
#include <string>
#include <map>
#include <iostream>
//#include <libmtp.h>
#include "jpmp_manager_DeviceManager.h"
#include "devicemanager.h"
#include "usbdevice.h"
#include "jsyncnotifier.h"
#include "usbdevmanapp.h"

#include <stdio.h>

devicemanager* pasmgr = NULL;
static usbdevman_app* theapp = usbdevman_app::get_instance();


#ifdef __cplusplus
extern "C" {
#endif



/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    setVerbose
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_setVerbose(JNIEnv *, jobject, jint verbose){
	theapp->set_verbose( ((verbose>0) ? 1 : 0) ); 
	return 0;
}

/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    createInstance
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_createInstance(JNIEnv *env, jobject obj){
	theapp->set_verbose(0); 
	CoInitialize(NULL);
	pasmgr = new devicemanager();	
	return 0;
}

/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    releaseInstance
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_releaseInstance(JNIEnv *env, jobject obj){
	if (pasmgr != NULL){
		delete(pasmgr);
	}
	CoUninitialize();
	if (theapp){
		theapp->release_instance();
	}
	return 0;
}


/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    detectDevices
 * Signature: (Ljpmp/manager/DeviceManager;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_detectDevices(JNIEnv *env, jobject obj){
	if(pasmgr == NULL) return 1;
	int ret = pasmgr->refresh_devices();
	
	jclass usbdevClass = env->FindClass( "jpmp/device/UsbDevice" );
	if (usbdevClass == NULL){
		if ( theapp->is_verbose() ){
			std::cout << "env->FindClass(\"jpmp/device/UsbDevice\" returns NULL" << std::endl;  
		}
		return 1;
	}
	
	jmethodID midConstr = env->GetMethodID(usbdevClass, "<init>", "()V");
	if (midConstr == NULL){
		if ( theapp->is_verbose() ){
			std::cout << "env->GetMethodID on <init> returns NULL" << std::endl;  
		}
		return 1;
	}
	
	devicemanager::UsbDevicesInfo listDevices;
	ret = pasmgr->fill_devicesinfo(&listDevices);
	if ( ret == 0 &&  listDevices.size()>0){
		devicemanager::UsbDevicesInfo::iterator it;
		
		jclass dmClass =env->GetObjectClass(obj);
		jmethodID midAdd = env->GetMethodID(dmClass, "addDevice", "(Ljpmp/device/UsbDevice;)V");
		if (midAdd == NULL){
			if ( theapp->is_verbose() ){
				std::cout << "env->GetMethodID on addDevice returns NULL" << std::endl;  
			}
			return 1;
		} else {
			if ( theapp->is_verbose() ){
				std::cout << "env->GetMethodID on addDevice returns method" << std::endl;  
			}
		}
		
		for(it = listDevices.begin(); it != listDevices.end();++it ){
			
			usbdevice_info usbdev = it->second;
			
			jobject jdev = env->NewObject(usbdevClass,midConstr);
			if (jdev == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->NewObject on <init> returns NULL" << std::endl;  
				}
				return 1;
			}
			
			// canonical name
			jmethodID midTemp = env->GetMethodID(usbdevClass, "setCanonical", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setCanonical returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_canonical().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_canonical().c_str());
			
			// serial number
			midTemp = env->GetMethodID(usbdevClass, "setSerial", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setSerial returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_serial().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_serial().c_str());

			// product name
			midTemp = env->GetMethodID(usbdevClass, "setName", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setName returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_name().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_name().c_str());
			
			// manufacturer
			midTemp = env->GetMethodID(usbdevClass, "setManufacturer", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setManufacturer returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_manufacturer().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_manufacturer().c_str());
			
			// video folder
			midTemp = env->GetMethodID(usbdevClass, "setVideoFolder", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setVideoFolder returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_videodir().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_videodir().c_str());
			
			// audio folder
			midTemp = env->GetMethodID(usbdevClass, "setAudioFolder", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setAudioFolder returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_audiodir().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_audiodir().c_str());
			
			// picture folder
			midTemp = env->GetMethodID(usbdevClass, "setPictureFolder", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setPictureFolder returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_pictdir().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_pictdir().c_str());
			
			// vendor id
			midTemp = env->GetMethodID(usbdevClass, "setVid", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setVid returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_vid().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_vid().c_str());

			midTemp = env->GetMethodID(usbdevClass, "setVendorid", "(I)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setVendorid returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,usbdev.get_vendorid());
			
			// product id
			midTemp = env->GetMethodID(usbdevClass, "setPid", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setPid returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_pid().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_pid().c_str());
			
			midTemp = env->GetMethodID(usbdevClass, "setProductid", "(I)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setProductid returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,usbdev.get_productid());
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_pid().c_str());
			
			
			// mount point
			midTemp = env->GetMethodID(usbdevClass, "setMountPoint", "(Ljava/lang/String;)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setMountPoint returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,env->NewStringUTF(CW2A(usbdev.get_mountpoint().c_str())));
			//env->CallVoidMethod(jdev,midTemp,usbdev->get_mountpoint().c_str());
			
			// usb index
			midTemp = env->GetMethodID(usbdevClass, "setUsbIndex", "(I)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setUsbIndex returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,usbdev.get_idx());
			
			// total size
			midTemp = env->GetMethodID(usbdevClass, "setTotalSize", "(J)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setTotalSize returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,usbdev.get_totalsize());
			
			// free size
			midTemp = env->GetMethodID(usbdevClass, "setFreeSize", "(J)V");
			if (midTemp == NULL){
				if ( theapp->is_verbose() ){
					std::cout << "env->GetMethodID on setTotalSize returns NULL" << std::endl;  
				}
				return 1;
			}
			env->CallVoidMethod(jdev,midTemp,usbdev.get_freesize());
			
			// device mode
			if (usbdev.get_devicemode() == usbdevice::MODE_MSC){
				midTemp = env->GetMethodID(usbdevClass, "setMscMode", "()V");
				if (midTemp == NULL){
					if ( theapp->is_verbose() ){
						std::cout << "env->GetMethodID on setMscMode returns NULL" << std::endl;  
					}
					return 1;
				}
				env->CallVoidMethod(jdev,midTemp);
			}
			if (usbdev.get_devicemode() == usbdevice::MODE_MTP){
				midTemp = env->GetMethodID(usbdevClass, "setMtpMode", "()V");
				if (midTemp == NULL){
					if ( theapp->is_verbose() ){
						std::cout << "env->GetMethodID on setMtpMode returns NULL" << std::endl;  
					}
					return 1;
				}
				env->CallVoidMethod(jdev,midTemp);
			}
			
			
			env->CallVoidMethod(obj,midAdd,jdev);
			
		}
	}

	
	
	
	
	
	/*
	jmethodID midDump = env->GetMethodID(usbdevClass, "toString", "()Ljava/lang/String;");
	if (midDump == NULL){
		if ( theapp->is_verbose() ){
			std::cout << "env->GetMethodID on dump returns NULL" << std::endl;  
		}
		return 1;
	}
	//jstring tt = env->CallVoidMethod(jdev,midDump);
	jstring tt = (jstring)env->CallObjectMethod(jdev,midDump);
	const char* cpptt = env->GetStringUTFChars(tt,0); 
	std::cout << cpptt << std::endl;
	env->ReleaseStringUTFChars(tt,cpptt);
	*/
	
	return ret;
}

/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    checkDevice
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_checkDevice(JNIEnv *env, jobject jdev, jstring jdevid){
	if(pasmgr == NULL) return 1;
	
	const char* devid = env->GetStringUTFChars(jdevid,0);
	std::wstring deviceid = CA2W(devid);
	if ( theapp->is_verbose() ){
		std::wcout << L"Check device :" << deviceid.c_str() << std::endl;
	}
	env->ReleaseStringUTFChars(jdevid,devid);
	
	int ret = pasmgr->check_device(deviceid);
	
	return ret;
}

/*
 * Class:     jpmp_manager_DeviceManager
 * Method:    releaseDevice
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_manager_DeviceManager_releaseDevice(JNIEnv *env, jobject jdev, jstring jdevid){
	if(pasmgr == NULL) return 1;
	
	const char* devid = env->GetStringUTFChars(jdevid,0);
	std::wstring deviceid = CA2W(devid);
	env->ReleaseStringUTFChars(jdevid,devid);
	
	int ret = pasmgr->release_device(deviceid);
	
	jclass dmClass =env->GetObjectClass(jdev);
	jmethodID midRemove = env->GetMethodID(dmClass, "removeDevice", "(Ljava/lang/String;)V");
	if (midRemove == NULL){
		if ( theapp->is_verbose() ){
			std::cout << "env->GetMethodID on removeDevice returns NULL" << std::endl;  
		}
		return 1;
	} else {
		if ( theapp->is_verbose() ){
			std::cout << "env->GetMethodID on removeDevice returns method" << std::endl;  
		}
	}

	env->CallVoidMethod(jdev,midRemove,jdevid);
	
	return ret;
}

#ifdef __cplusplus
}
#endif

