/*
 *	\file jsyncnotifier.h
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
#ifndef JSYNCNOTIFIER_H_
#define JSYNCNOTIFIER_H_

#include "isyncnotifier.h"

#include <jni.h>

class jsyncnotifier : public isyncnotifier
{
public:
	jsyncnotifier();
	virtual ~jsyncnotifier();
	
	virtual long notify_begin(unsigned __int64 size);
	virtual long notify_current(unsigned __int64 pos);
	virtual long notify_end();
	virtual int  is_abort();
	
	void setJniObject(jobject* newobj){obj = newobj;};
	void setJniEnv(JNIEnv *newenv){env = newenv;};

protected:	
	(unsigned __int64 totalSize;
	jobject* obj;
	JNIEnv *env;
};

#endif /*JSYNCNOTIFIER_H_*/
