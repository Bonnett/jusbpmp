/*
 *	\file jsyncnotifier.cpp
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
#include "jsyncnotifier.h"
#include "usbdevmanapp.h"


static usbdevman_app* theapp = usbdevman_app::get_instance();

jsyncnotifier::jsyncnotifier(){
	totalSize = 0;
}

jsyncnotifier::~jsyncnotifier(){
}

long jsyncnotifier::notify_begin(unsigned __int64 size){
	long lr = 0;
	totalSize = size;
	if (theapp->is_verbose() ){
		std::cout<< "notify_begin" << std::endl;
	}
	jclass classJava = env->GetObjectClass((*obj)); 
	jmethodID methodId = env->GetMethodID(classJava, "notifyBegin", "(J)V"); 
	if (methodId == NULL){
		return -1;
	} else {
		env->CallVoidMethod(*obj, methodId, (jlong)size);
	}
	return lr;
}

long jsyncnotifier::notify_current(unsigned __int64 pos){
	long lr = 0;
	//std::cout<< "c++ : notify_current : " << pos << std::endl;
	
	jclass classJava = env->GetObjectClass((*obj)); 
	jmethodID methodId = env->GetMethodID(classJava, "notifyCurrent", "(J)V"); 
	if (methodId == NULL){
		return -1;
	} else {
		env->CallVoidMethod(*obj, methodId, (jlong)pos);
	}
	return lr;
}

long jsyncnotifier::notify_end(){
	long lr = 0;
	if (theapp->is_verbose() ){
		std::cout<< "notify_end" << std::endl;
	}
	jclass classJava = env->GetObjectClass((*obj)); 
	jmethodID methodId = env->GetMethodID(classJava, "notifyEnd", "()V"); 
	if (methodId == NULL){
		return -1;
	} else {
		env->CallVoidMethod(*obj, methodId, 0);
	}
	return lr;
}

int  jsyncnotifier::is_abort(){
	long lr = 0;
	jclass classJava = env->GetObjectClass((*obj)); 
	jmethodID methodId = env->GetMethodID(classJava, "getAbort", "()Z"); 
	if (methodId == NULL){
		return 0;
	} else {
		return env->CallBooleanMethod(*obj, methodId, 0);
	}
	return lr;
}
