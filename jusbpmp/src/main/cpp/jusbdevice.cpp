/*
 *	\file jusbdevice.h
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
#include <algorithm>
#include <fcntl.h>
#ifndef _WINDOWS
#include <libmtp.h>
#include <dirent.h>
#endif
#include "jpmp_device_UsbDevice.h"
#include "devicemanager.h"
#include "jsyncnotifier.h"
#include "usbdevmanapp.h"
#include "usbdevice.h"
#include "helperlib.h"
#include "defines.h"
#include "filemetadata.h"


extern devicemanager* pasmgr;

static usbdevman_app* theapp = usbdevman_app::get_instance();


long get_internaldevice(JNIEnv * env, jobject jdev, usbdevice*& internaldev){
	jclass jdevcls = env->GetObjectClass(jdev);
	
	jmethodID midCano = env->GetMethodID(jdevcls, "getCanonical", "()Ljava/lang/String;");
	if (midCano == NULL){
		if ( theapp->is_verbose() ){
			std::cout << "env->GetMethodID on getCanonical returns NULL" << std::endl;  
		}
		return jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}
	
	jstring jdevid = (jstring)env->CallObjectMethod(jdev,midCano);
	
	const char* devid = env->GetStringUTFChars(jdevid,0);
	internaldev = (usbdevice*)pasmgr->get_devicebyid(std::wstring(CA2W(devid)));
	env->ReleaseStringUTFChars(jdevid,devid);
	return jpmp_device_UsbDevice_ERROR_SUCCESS;;
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    existFile
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_existFile (JNIEnv * env, jobject jdev, jstring path){
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	
	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* afpath = env->GetStringUTFChars(path,0);
	LPCWSTR fpath = CA2W(afpath);
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring devicepath = fpath;
		std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );
		std::wstring fulldevicepath = mountpoint + devicepath;

		bool b = helperlib::exist_file_msc(fulldevicepath);
		if (b) {
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		} else {
			ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
		}
			
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			bool b = usbdev->exist_file_mtp(fpath);
			if (b) {
				ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
			} else {
				ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}

	env->ReleaseStringUTFChars(path,afpath);

	return ret;	
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    deleteFile
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_deleteFile (JNIEnv * env, jobject jdev, jstring path){
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);
	std::wstring sfpath = CA2W(fpath);

	if (sfpath.length()==0 || sfpath == L"/"){
		env->ReleaseStringUTFChars(path,fpath);
		return jpmp_device_UsbDevice_ERROR_WRONG_PARAMETER;
	}
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring devicepath = sfpath;
		std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );
		std::wstring fulldevicepath = mountpoint + devicepath;

		if ( DeleteFile(fulldevicepath.c_str()) ) {
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		} else {
			ret = jpmp_device_UsbDevice_ERROR_UNKNOWN;
		}
			
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			bool b = usbdev->delete_mtpitem(sfpath);
			if (b) {
				ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
			} else {
				ret = jpmp_device_UsbDevice_ERROR_UNKNOWN;
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}

	env->ReleaseStringUTFChars(path,fpath);
	return ret;	
	
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    existFolder
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_existFolder(JNIEnv * env, jobject jdev, jstring path){
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* afpath = env->GetStringUTFChars(path,0);
	LPCWSTR fpath = CA2W(afpath);
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring devicepath = fpath;
		std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );
		std::wstring fulldevicepath = mountpoint + devicepath;

		bool b = helperlib::exist_folder_msc(fulldevicepath);
		if (b) {
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		} else {
			ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
		}
			
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			bool b = usbdev->exist_folder_mtp(fpath);
			if (b) {
				ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
			} else {
				ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}

	env->ReleaseStringUTFChars(path,afpath);
	return ret;	
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    deleteFolder
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_deleteFolder(JNIEnv * env, jobject jdev, jstring path){
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);
	std::wstring sfpath = CA2W(fpath);

	if (sfpath.length()==0 || sfpath == L"/"){
		env->ReleaseStringUTFChars(path,fpath);
		return jpmp_device_UsbDevice_ERROR_WRONG_PARAMETER;
	}
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring devicepath = sfpath;
		std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );
		std::wstring fulldevicepath = mountpoint + devicepath;

		if (RemoveDirectory(fulldevicepath.c_str()) != 0) {
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		} else {
			ret = jpmp_device_UsbDevice_ERROR_UNKNOWN;
		}
			
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			bool b = usbdev->delete_mtpitem(sfpath);
			if (b) {
				ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
			} else {
				ret = jpmp_device_UsbDevice_ERROR_UNKNOWN;
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}

	env->ReleaseStringUTFChars(path,fpath);
	return ret;	
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    createFolder
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_createFolder(JNIEnv * env, jobject jdev, jstring path){
	
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);
	std::wstring sfpath = CA2W(fpath);
	if (sfpath.length()==0 || sfpath == L"/"){
		env->ReleaseStringUTFChars(path,fpath);
		return jpmp_device_UsbDevice_ERROR_WRONG_PARAMETER;
	}
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring devicepath = sfpath;
		std::replace( devicepath.begin(), devicepath.end(), L'/', L'\\' );
		std::wstring fulldevicepath = mountpoint + devicepath;

		BOOL b = CreateDirectory(fulldevicepath.c_str(), NULL);
		if (b) {
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		} else {
			ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
		}
		
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			bool b = usbdev->create_folder_mtp(sfpath);
			if (b) {
				ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
			} else {
				ret = jpmp_device_UsbDevice_ERROR_FILE_NOTFOUND;
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}

	env->ReleaseStringUTFChars(path,fpath);
	return ret;	
	
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    getMetadata
 * Signature: (Ljava/lang/String;Ljpmp/device/FileMetadata;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_getMetadata(JNIEnv * env, jobject jdev, jstring path, jobject metadata){
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	if (metadata == NULL){
		return jpmp_device_UsbDevice_ERROR_SUCCESS;
	}
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	
	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		return jpmp_device_UsbDevice_ERROR_MSC_NOTAVAILABLE;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);
	CComPtr<IWMDMMetaData> spMetadata;
	ret = usbdev->get_metadata(std::wstring(CA2W(fpath)),&spMetadata);
	if (ret != 0){
		SAFE_COMPTR_RELEASE(spMetadata);
		return jpmp_device_UsbDevice_ERROR_METADATA_PROPERTY;
	}
	if (spMetadata.p == NULL){
		return jpmp_device_UsbDevice_ERROR_METADATA_PROPERTY;
	}
	
	HRESULT hr = S_OK;
	std::wstring title = L"";
	std::wstring album = L"";
	std::wstring artist = L"";
	std::wstring genre = L"";
	UINT rating = 0;


	

	

	try{
		LPCWSTR props[] = {g_wszWMDMTitle, g_wszWMDMAlbumArt,g_wszWMDMAuthor,g_wszWMDMGenre,g_wszWMDMUserRating};
		// query for each metadata value.
		BYTE* pVal = NULL;
		UINT len = 0;
		WMDM_TAG_DATATYPE datatype;
		hr = spMetadata->QueryByName(g_wszWMDMTitle, &datatype, &pVal, &len);
		if(hr == S_OK) {
			title = (LPWSTR)pVal;
			CoTaskMemFree(pVal);
			pVal = NULL;
		}
		hr = spMetadata->QueryByName(g_wszWMDMAuthor, &datatype, &pVal, &len);
		if(hr == S_OK) {
			artist = (LPWSTR)pVal;
			if ( theapp->is_verbose() ){
				std::wcout << "Metadata artist : " << artist << std::endl;
			}
			CoTaskMemFree(pVal);
			pVal = NULL;
		}
		hr = spMetadata->QueryByName(g_wszWMDMAlbumArt, &datatype, &pVal, &len);
		if(hr == S_OK) {
			album = (LPWSTR)pVal;
			CoTaskMemFree(pVal);
			pVal = NULL;
		}
		hr = spMetadata->QueryByName(g_wszWMDMGenre, &datatype, &pVal, &len);
		if(hr == S_OK) {
			genre = (LPWSTR)pVal;
			CoTaskMemFree(pVal);
			pVal = NULL;
		}

		hr = spMetadata->QueryByName(g_wszWMDMUserRating, &datatype, &pVal, &len);
		if(hr == S_OK) {
			rating = (*pVal);
			CoTaskMemFree(pVal);
		}

	} catch(HRESULT hrex){
		hr = hrex;
	}

	SAFE_COMPTR_RELEASE(spMetadata);
	hr = S_OK;
		
	try{

		jclass jmetacls = env->GetObjectClass(metadata);
		jmethodID midSetter = env->GetMethodID(jmetacls, "setTitle", "(Ljava/lang/String;)V");
		if (midSetter == NULL){
			throw 1;
		}
		env->CallObjectMethod(metadata,midSetter,env->NewStringUTF(CW2A(title.c_str())));
		
		midSetter = env->GetMethodID(jmetacls, "setAlbum", "(Ljava/lang/String;)V");
		if (midSetter == NULL){
			throw 2;
		}
		env->CallObjectMethod(metadata,midSetter,env->NewStringUTF(CW2A(album.c_str())));
		
		midSetter = env->GetMethodID(jmetacls, "setArtist", "(Ljava/lang/String;)V");
		if (midSetter == NULL){
			throw 3;
		}
		env->CallObjectMethod(metadata,midSetter,env->NewStringUTF(CW2A(artist.c_str())));
		
		midSetter = env->GetMethodID(jmetacls, "setGenre", "(Ljava/lang/String;)V");
		if (midSetter == NULL){
			throw 3;
		}
		env->CallObjectMethod(metadata,midSetter,env->NewStringUTF(CW2A(genre.c_str())));
		
		midSetter = env->GetMethodID(jmetacls, "setRating", "(I)V");
		if (midSetter == NULL){
			throw 3;
		}
		env->CallObjectMethod(metadata,midSetter,rating);
		
		ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		
	} catch(int err){
		std::cout << "Metadata check error " << err << std::endl;
		
		ret = jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}

	env->ReleaseStringUTFChars(path,fpath);
	return ret;
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    updateMetadata
 * Signature: (Ljava/lang/String;Ljpmp/device/FileMetadata;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_updateMetadata(JNIEnv * env, jobject jdev, jstring path, jobject metadata){

	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	if (metadata == NULL){
		return jpmp_device_UsbDevice_ERROR_SUCCESS;
	}
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		return jpmp_device_UsbDevice_ERROR_MSC_NOTAVAILABLE;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);

	filemetadata* pmeta = NULL;
	filemetadata meta;

	try{
		
		if (metadata != NULL){
			
			jclass jmetacls = env->GetObjectClass(metadata);
			jmethodID midTitle = env->GetMethodID(jmetacls, "getTitle", "()Ljava/lang/String;");
			if (midTitle == NULL){
				throw 1;
			}
			jstring jtitle = (jstring)env->CallObjectMethod(metadata,midTitle);
			const char* title = env->GetStringUTFChars(jtitle,0);
			meta.title = CA2W(title);
			env->ReleaseStringUTFChars(jtitle,title);
			
			jmethodID midAlbum = env->GetMethodID(jmetacls, "getAlbum", "()Ljava/lang/String;");
			if (midAlbum == NULL){
				throw 2;
			}
			jstring jalbum = (jstring)env->CallObjectMethod(metadata,midAlbum);
			const char* album = env->GetStringUTFChars(jalbum,0);
			meta.album = CA2W(album);
			env->ReleaseStringUTFChars(jalbum,album);
			
			jmethodID midArtist = env->GetMethodID(jmetacls, "getArtist", "()Ljava/lang/String;");
			if (midArtist == NULL){
				throw 3;
			}
			jstring jartist = (jstring)env->CallObjectMethod(metadata,midArtist);
			const char* artist = env->GetStringUTFChars(jartist,0);
			meta.artist = CA2W(artist);
			env->ReleaseStringUTFChars(jartist,artist);
			
			jmethodID midGenre = env->GetMethodID(jmetacls, "getGenre", "()Ljava/lang/String;");
			if (midGenre == NULL){
				throw 4;
			}
			jstring jgenre = (jstring)env->CallObjectMethod(metadata,midGenre);
			const char* genre = env->GetStringUTFChars(jgenre,0);
			meta.genre = CA2W(genre);
			env->ReleaseStringUTFChars(jgenre,genre);
			
			jmethodID midRate = env->GetMethodID(jmetacls, "getRating", "()I");
			if (midRate == NULL){
				throw 5;
			}
			jint jrate = env->CallIntMethod(metadata,midRate);
			meta.rating = jrate;

			pmeta = &meta;
		}
		
	} catch(int err){
		std::cout << "Metadata check error " << err << std::endl;
		ret = jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}

	if (theapp->is_verbose()){
		meta.dump();
	}

	if (pmeta != NULL){
		ret = usbdev->update_metadata(std::wstring(CA2W(fpath)),pmeta);
	}

	env->ReleaseStringUTFChars(path,fpath);

	return ret;
}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    getFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljpmp/notifier/IDeviceTransferNotifier;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_getFile(JNIEnv * env, jobject jdev, jstring localdest, jstring pmpsrc, jobject notif){
	
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	const char* fdest = env->GetStringUTFChars(localdest,0);
	const char* fsrc = env->GetStringUTFChars(pmpsrc,0);
	
	jsyncnotifier ntf;
	ntf.setJniObject(&notif);
	ntf.setJniEnv(env);
	
	ret = usbdev->read_file(std::wstring(CA2W(fsrc)), std::wstring(CA2W(fdest)), &ntf);
	
	env->ReleaseStringUTFChars(localdest,fdest);
	env->ReleaseStringUTFChars(pmpsrc,fsrc);

	return ret;
	
}


/*
 * Class:     jpmp_device_UsbDevice
 * Method:    sendFile
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljpmp/notifier/IDeviceTransferNotifier;Ljpmp/device/FileMetadata;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_sendFile
  (JNIEnv *env, jobject jdev, jstring srcfile, jstring destfile, jobject notif, jobject metadata){
  
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	jlong ret = jpmp_device_UsbDevice_ERROR_SUCCESS;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}

	const char* devsrc = env->GetStringUTFChars(srcfile,0);
	const char* devdest = env->GetStringUTFChars(destfile,0);


	jsyncnotifier ntf;
	ntf.setJniObject(&notif);
	ntf.setJniEnv(env);

	filemetadata* pmeta = NULL;
	filemetadata meta;

	try{
		
		if (metadata != NULL){
			
			jclass jmetacls = env->GetObjectClass(metadata);
			jmethodID midTitle = env->GetMethodID(jmetacls, "getTitle", "()Ljava/lang/String;");
			if (midTitle == NULL){
				throw 1;
			}
			jstring jtitle = (jstring)env->CallObjectMethod(metadata,midTitle);
			const char* title = env->GetStringUTFChars(jtitle,0);
			meta.title = CA2W(title);
			env->ReleaseStringUTFChars(jtitle,title);
			
			jmethodID midAlbum = env->GetMethodID(jmetacls, "getAlbum", "()Ljava/lang/String;");
			if (midAlbum == NULL){
				throw 2;
			}
			jstring jalbum = (jstring)env->CallObjectMethod(metadata,midAlbum);
			const char* album = env->GetStringUTFChars(jalbum,0);
			meta.album = CA2W(album);
			env->ReleaseStringUTFChars(jalbum,album);
			
			jmethodID midArtist = env->GetMethodID(jmetacls, "getArtist", "()Ljava/lang/String;");
			if (midArtist == NULL){
				throw 3;
			}
			jstring jartist = (jstring)env->CallObjectMethod(metadata,midArtist);
			const char* artist = env->GetStringUTFChars(jartist,0);
			meta.artist = CA2W(artist);
			env->ReleaseStringUTFChars(jartist,artist);
			
			jmethodID midGenre = env->GetMethodID(jmetacls, "getGenre", "()Ljava/lang/String;");
			if (midGenre == NULL){
				throw 4;
			}
			jstring jgenre = (jstring)env->CallObjectMethod(metadata,midGenre);
			const char* genre = env->GetStringUTFChars(jgenre,0);
			meta.genre = CA2W(genre);
			env->ReleaseStringUTFChars(jgenre,genre);
			
			jmethodID midRate = env->GetMethodID(jmetacls, "getRating", "()I");
			if (midRate == NULL){
				throw 5;
			}
			jint jrate = env->CallIntMethod(metadata,midRate);
			meta.rating = jrate;

			pmeta = &meta;
		}
		
	} catch(int err){
		std::cout << "Metadata check error " << err << std::endl;
		ret = jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}

	ret = usbdev->transfer_file(
				std::wstring(CA2W(devsrc)), 
				std::wstring(CA2W(devdest)), 
				&ntf,
				pmeta);


	//env->ReleaseStringUTFChars(jdevid,devid);
	env->ReleaseStringUTFChars(srcfile,devsrc);
	env->ReleaseStringUTFChars(destfile,devdest);



	return ret;

}

/*
 * Class:     jpmp_device_UsbDevice
 * Method:    parseFolder
 * Signature: (Ljava/lang/String;Ljpmp/notifier/IParseTreeNotifier;)J
 */
JNIEXPORT jlong JNICALL Java_jpmp_device_UsbDevice_parseFolder(JNIEnv * env, jobject jdev, jstring path, jobject notif){
	
	if(pasmgr == NULL) return jpmp_device_UsbDevice_ERROR_INIT_DEViCEMANAGER;
	
	jlong ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;

	usbdevice* usbdev;
	get_internaldevice(env,jdev,usbdev);
	
	if (usbdev == NULL){
		return jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
	}
	
	if (notif == NULL){
		return jpmp_device_UsbDevice_ERROR_WRONG_PARAMETER;
	}
	jclass jnotifcls = env->GetObjectClass(notif);
	jmethodID midAddFile = env->GetMethodID(jnotifcls, "addFile", "(Ljava/lang/String;Ljava/lang/String;)J");
	if (midAddFile == NULL){
		return jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}
	jmethodID midAddFolder = env->GetMethodID(jnotifcls, "addFolder", "(Ljava/lang/String;Ljava/lang/String;)J");
	if (midAddFolder == NULL){
		return jpmp_device_UsbDevice_ERROR_INVOKE_METHOD;
	}
	
	const char* fpath = env->GetStringUTFChars(path,0);
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		usbdevice::mtp_itemlist files;
		usbdevice::mtp_itemlist folders;
		ret = usbdev->parse_folder_msc(std::wstring(CA2W(fpath)),&files,&folders);
		
		if ( theapp->is_verbose() ){
			std::cout << "Found " << files.size() << " files " <<std::endl;
			std::cout << "Found " << folders.size() << " folders " <<std::endl;
		}
		for (size_t i=0; i<files.size(); i++){
			usbdevice::mtp_iteminfo item = files.at(i);
			std::wstring fname = item.first;
			std::wstring itemid = item.second;
			env->CallObjectMethod(notif,midAddFile,env->NewStringUTF(CW2A(fname.c_str())),env->NewStringUTF(CW2A(itemid.c_str())));
		}
		for (size_t i=0; i<folders.size(); i++){
			usbdevice::mtp_iteminfo item = folders.at(i);
			std::wstring fname = item.first;
			std::wstring itemid = item.second;
			env->CallObjectMethod(notif,midAddFolder,env->NewStringUTF(CW2A(fname.c_str())),env->NewStringUTF(CW2A(itemid.c_str())));
		}
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			usbdevice::mtp_itemlist files;
			usbdevice::mtp_itemlist folders;
			ret = usbdev->parse_folder_mtp(std::wstring(CA2W(fpath)),&files,&folders);
			
			if ( theapp->is_verbose() ){
				std::cout << "Found " << files.size() << " files " <<std::endl;
				std::cout << "Found " << folders.size() << " folders " <<std::endl;
			}
			for (size_t i=0; i<files.size(); i++){
				usbdevice::mtp_iteminfo item = files.at(i);
				std::wstring fname = item.first;
				std::wstring itemid = item.second;
				env->CallObjectMethod(notif,midAddFile,env->NewStringUTF(CW2A(fname.c_str())),env->NewStringUTF(CW2A(itemid.c_str())));
			}
			for (size_t i=0; i<folders.size(); i++){
				usbdevice::mtp_iteminfo item = folders.at(i);
				std::wstring fname = item.first;
				std::wstring itemid = item.second;
				env->CallObjectMethod(notif,midAddFolder,env->NewStringUTF(CW2A(fname.c_str())),env->NewStringUTF(CW2A(itemid.c_str())));
			}
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}


#ifndef _WINDOWS	
	
	if (usbdev->get_devicemode() == usbdevice::MODE_MSC){
		std::wstring mountpoint = usbdev->get_mountpoint();
		std::wstring fullpath = mountpoint + fpath;
		
		dirent 	*dent		= NULL;
		DIR 	*directory	= NULL;

		directory=opendir(fullpath.c_str());
		if(directory == NULL) {
			if ( theapp->is_verbose() ){
    			std::cerr << "Couldn't open directory" << std::endl;
    		}
    		ret = jpmp_device_UsbDevice_ERROR_FOLDER_NOTFOUND;
  		} else {

			// Loop for content
			while( (dent = readdir(directory)) ){
				std::wstring fname = dent->d_name;
				if (fname == "." || fname == "..") continue;
				std::wstring fullsubpath = fullpath + "/" + dent->d_name;
				uint32_t itemid = 0;
				
				if (helperlib::exist_file_msc(fullsubpath)){
					// this is a file
					env->CallObjectMethod(notif,midAddFile,env->NewStringUTF(fname.c_str()),itemid);
				} else
				if (helperlib::exist_folder_msc(fullsubpath)){
					// this is a folder
					env->CallObjectMethod(notif,midAddFolder,env->NewStringUTF(fname.c_str()),itemid);
				}
			}
			closedir(directory);
			ret = jpmp_device_UsbDevice_ERROR_SUCCESS;
		}
		
	} else {
		if (usbdev->get_devicemode() == usbdevice::MODE_MTP){
			usbdevice::mtp_itemlist files;
			usbdevice::mtp_itemlist folders;
			ret = usbdev->parse_folder_mtp(fpath,&files,&folders);
			
			if ( theapp->is_verbose() ){
    			std::cout << "Found " << files.size() << " files " <<std::endl;
    			std::cout << "Found " << folders.size() << " folders " <<std::endl;
    		}
			for (size_t i=0; i<files.size(); i++){
				usbdevice::mtp_iteminfo item = files.at(i);
				std::wstring fname = item.first;
				uint32_t itemid = item.second;
				env->CallObjectMethod(notif,midAddFile,env->NewStringUTF(fname.c_str()),itemid);
			}
			for (size_t i=0; i<folders.size(); i++){
				usbdevice::mtp_iteminfo item = folders.at(i);
				std::wstring fname = item.first;
				uint32_t itemid = item.second;
				env->CallObjectMethod(notif,midAddFolder,env->NewStringUTF(fname.c_str()),itemid);
			}
			
		} else{
			ret = jpmp_device_UsbDevice_ERROR_DEVICE_NOTMOUNTED;
		}
	}
#endif

	env->ReleaseStringUTFChars(path,fpath);

	return ret;	
	
}
