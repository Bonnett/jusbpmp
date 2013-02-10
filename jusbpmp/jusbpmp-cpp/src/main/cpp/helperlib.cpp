/*
 *	\file helperlib.cpp
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
#include "helperlib.h"
#include <iostream>
#include <TCHAR.H>
#include <sys/types.h>
#include <sys/stat.h> 
#include "usbdevmanapp.h"

static usbdevman_app* theapp = usbdevman_app::get_instance();


bool helperlib::exist_file_msc(std::wstring path){
	
	struct _stat sb;
	if ( _wstat(path.c_str(), &sb) == -1 ) {
		if (theapp->is_verbose()){
			std::wcerr << L"File not found " << path.c_str() << std::endl;
		}
		return false;
	}
	if ( sb.st_mode & S_IFREG ) {
		return true;
	}
	if (theapp->is_verbose()){
		std::wcerr << L"Path found " << path.c_str() << L", but not a file" << std::endl;
	}
	return false;
}


bool helperlib::exist_folder_msc(std::wstring path){
	
	struct _stat sb;
	
	if ( _wstat(path.c_str(), &sb) == -1 ) {
		if (theapp->is_verbose()){
			std::wcerr << L"Folder not found " << path.c_str() << std::endl;
		}
		return false;
	}
	if ( sb.st_mode & S_IFDIR ) {
		return true;
	}
	if (theapp->is_verbose()){
		std::wcerr << L"Path found " << path.c_str() << L", but not a folder" << std::endl;
	}
	return false;
}

uint64_t helperlib::obtain_filesize(std::wstring path){
	
	uint64_t filesize;
	struct _stat sb;
	if ( _wstat(path.c_str(), &sb) == -1 ) {
		fprintf(stderr, "%s: ", path.c_str());
		perror("stat");
		return false;
	}

	filesize = (uint64_t) sb.st_size;

	return filesize;
}

#if 0
/* Find the file type based on extension */
LIBMTP_filetype_t helperlib::find_filetype (std::wstring filename) {
	std::wstring ftype;
	LIBMTP_filetype_t filetype;

	size_t pos = filename.find_last_of(".");
	// This accounts for the case with a filename without any "." (period).
	if (pos == std::wstring::npos) {
		ftype = "";
	} else {
		ftype = filename.substr(pos+1);
	}
	
	if( ftype.length()>0){
		std::transform(ftype.begin(), ftype.end(), ftype.begin(), tolower);
	}

	/* This need to be kept constantly updated as new file types arrive. */
	if (ftype == "wav") {
		filetype = LIBMTP_FILETYPE_WAV;
	} else if (ftype == "mp3") {
		filetype = LIBMTP_FILETYPE_MP3;
	} else if (ftype == "wma") {
		filetype = LIBMTP_FILETYPE_WMA;
	} else if (ftype == "ogg") {
		filetype = LIBMTP_FILETYPE_OGG;
	} else if (ftype == "mp4") {
		filetype = LIBMTP_FILETYPE_MP4;
	} else if (ftype == "wmv") {
		filetype = LIBMTP_FILETYPE_WMV;
	} else if (ftype == "avi") {
		filetype = LIBMTP_FILETYPE_AVI;
	} else if (ftype == "mpeg" || ftype == "mpg") {
		filetype = LIBMTP_FILETYPE_MPEG;
  	} else if (ftype == "asf") {
		filetype = LIBMTP_FILETYPE_ASF;
	} else if (ftype == "qt" || ftype == "mov") {
		filetype = LIBMTP_FILETYPE_QT;
	} else if (ftype == "wma") {
		filetype = LIBMTP_FILETYPE_WMA;
	} else if (ftype == "jpg" || ftype == "jpeg") {
		filetype = LIBMTP_FILETYPE_JPEG;
	} else if (ftype == "jfif") {
		filetype = LIBMTP_FILETYPE_JFIF;
	} else if (ftype == "tif" || ftype == "tiff") {
		filetype = LIBMTP_FILETYPE_TIFF;
	} else if (ftype == "bmp") {
		filetype = LIBMTP_FILETYPE_BMP;
	} else if (ftype == "gif") {
		filetype = LIBMTP_FILETYPE_GIF;
	} else if (ftype == "pic" || ftype == "pict") {
		filetype = LIBMTP_FILETYPE_PICT;
	} else if (ftype == "png") {
		filetype = LIBMTP_FILETYPE_PNG;
	} else if (ftype == "wmf") {
		filetype = LIBMTP_FILETYPE_JPMPWINIMAGEFORMAT;
	} else if (ftype == "ics") {
		filetype = LIBMTP_FILETYPE_VCALENDAR2;
	} else if (ftype == "exe" || ftype == "com" ||
	     ftype == "bat" || ftype == "dll" ||
	     ftype == "sys") {
		filetype = LIBMTP_FILETYPE_WINEXEC;
	} else if (ftype == "aac") {
		filetype = LIBMTP_FILETYPE_AAC;
	} else if (ftype == "mp2") {
		filetype = LIBMTP_FILETYPE_MP2;
	} else if (ftype == "flac") {
		filetype = LIBMTP_FILETYPE_FLAC;
	} else if (ftype == "m4a") {
		filetype = LIBMTP_FILETYPE_M4A;
	} else if (ftype == "doc") {
		filetype = LIBMTP_FILETYPE_DOC;
	} else if (ftype == "xml") {
		filetype = LIBMTP_FILETYPE_XML;
	} else if (ftype == "xls") {
		filetype = LIBMTP_FILETYPE_XLS;
	} else if (ftype == "ppt") {
		filetype = LIBMTP_FILETYPE_PPT;
	} else if (ftype == "mht") {
		filetype = LIBMTP_FILETYPE_MHT;
	} else if (ftype == "jp2") {
		filetype = LIBMTP_FILETYPE_JP2;
	} else if (ftype == "jpx") {
		filetype = LIBMTP_FILETYPE_JPX;
	} else if (ftype == "bin") {
		filetype = LIBMTP_FILETYPE_FIRMWARE;
	} else {
		/* Tagging as unknown file type */
		filetype = LIBMTP_FILETYPE_UNKNOWN;
 	}
  
	return filetype;
}
#endif


int helperlib::split_string(const std::wstring& input, const std::wstring& delimiter, std::vector<std::wstring>& results, bool includeempties){
    
    size_t delimsize = delimiter.length();
    size_t isize = input.length();
    size_t numfound = 0;
    std::wstring temp = input;
    
    size_t ipos = 0;

    if( ( isize == 0 ) || ( delimsize == 0 ) ) {
        return 0;
    }

	// remove leading "/" if any
	if (temp[0] == '/'){
		temp = temp.substr(1);
	}
	
    ipos = temp.find (delimiter, 0);
    if (ipos == std::wstring::npos){
    	results.push_back(temp);
    	numfound++;
    	return numfound;
    }
    
    while (ipos != std::wstring::npos){
    	std::wstring token = temp.substr(0,ipos);
    	if (token.length()>0 || (token.length()==0 && includeempties)){
    		results.push_back(token);
    		numfound++;
    	}    	
    	temp = temp.substr(ipos+1);	
		ipos = temp.find (delimiter, 0);
    }
    results.push_back(temp);
    numfound++;

    return numfound;
}

