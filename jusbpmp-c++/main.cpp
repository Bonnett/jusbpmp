/*
 *	\file maincpp.h
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
#include <stdio.h>

#include <string>
#include <iostream>

#include "devicemanager.h"
#include "defines.h"
#include "usbdevmanapp.h"


//extern int is_verbose();
static const int verbose_level = 1;
static int is_verbose(){
	return verbose_level;
} 


int detect_devices_msc(){
	
	int retcode = 0;
	int numdev;
	
	char** pdevs 			= NULL;
	LibHalContext *ctxt 	= NULL;
	DBusConnection *conn 	= NULL;
	DBusError dbuserr;
	std::string volume_mount = "";
	
	if (usbdevman_app::get_instance()->is_verbose() ){
		printf("enter detect_devices\n");
	}
	
	try{
		if (is_verbose() ){
			printf("dbus_error_init (&dbuserr);\n");
		}
		dbus_error_init (&dbuserr);
		conn = dbus_bus_get (DBUS_BUS_SYSTEM, &dbuserr);
		if (conn == NULL){
			if (is_verbose() ){
				std::cout << "dbus_bus_get error : " << dbuserr.message << std::endl;
			}
			throw 1;
		}
	
		if (is_verbose() ){
			printf("libhal_ctx_new();\n");
		}
		ctxt = libhal_ctx_new();
		if (ctxt == NULL){
			if (is_verbose() ){
				std::cout << "libhal_ctx_new error " << std::endl;
			}
			throw 2;
		}
		
		libhal_ctx_set_dbus_connection (ctxt, conn);
		if (!libhal_ctx_init(ctxt, &dbuserr)){
			if (is_verbose() ){
				std::cout << "libhal_ctx_init error : " << dbuserr.message << std::endl;
			}
			throw 3;
		}
		
		/*pdevs = libhal_manager_find_device_string_match (ctxt,
							"info.linux.driver",
							"usb-storage",
							&numdev,
							NULL);*/
							
		pdevs = libhal_manager_find_device_string_match (ctxt,
							"info.category",
							"volume",
							&numdev,
							NULL);
							
							
		if (is_verbose() ){
			std::cout << "found : " << numdev << " compatible devices " << std::endl;
		}
							
		if (pdevs){
			int i;
			for (i=0;i<numdev;i++){
				if (is_verbose() ){
					std::cout << "found udi : " << pdevs[i] << std::endl;
				}
				LibHalVolume* volume = libhal_volume_from_udi(ctxt,pdevs[i]);
				if (volume != NULL){
					if (is_verbose() ){
						std::cout << "found volume udi : " << volume->udi <<std::endl;
					}
					if (volume->is_mounted){
						if (is_verbose() ){
							std::cout << "mounted at : " << volume->mount_point << std::endl;
						}
						const char* storageudi = libhal_volume_get_storage_device_udi (volume);
						if (is_verbose() ){
							std::cout << "storageudi = " << storageudi << std::endl;
						}
						LibHalDrive* drive = libhal_drive_from_udi(ctxt,storageudi);
						if (drive != NULL){
							if (is_verbose() ){
								std::cout << "serial = " << drive->serial << std::endl;
							}
							const char* physicaludi = libhal_drive_get_physical_device_udi (drive);
							if (is_verbose() ){
								std::cout << "physicaludi = " << physicaludi << std::endl;
							}
							
							if (libhal_device_property_exists(ctxt,physicaludi,"usb.vendor_id",NULL)){
								int prop = libhal_device_get_property_int(ctxt,physicaludi,"usb.vendor_id",NULL);
								if (is_verbose() ){
									std::cout << "usb.vendor_id = " << prop << std::endl;
								}
								/*if (prop != 0xe79 && prop != 0xfce){
									throw 4;
								}*/
								char vid[50];
								sprintf(vid, "0x%X", prop);
							}
							
						}else{
							std::cout << "drive is null " << std::endl;
						}
					}
				}else{
					std::cout << "volume is null " << std::endl;
				}
			}
		} else {
			if (is_verbose() ){
				std::cout << "no compatible device found " << std::endl;
			}
		}
		
	} catch(int err){
		retcode = err;
		if (is_verbose() ){
			std::cout << "detect_devices error = " << err << std::endl;
		}
	} catch(...){
		if (is_verbose() ){
			std::cout << "detect_devices error unknown " << std::endl;
		}
	}
	
	//clean-up
	if (pdevs){
		libhal_free_string_array(pdevs);
	}
	if(ctxt){
		libhal_ctx_shutdown(ctxt, &dbuserr);
		libhal_ctx_free(ctxt);
	}

	
	//std::cout << "exit detect_devices with code " << retcode << std::endl;
	return retcode;
}

int main(void){
	usbdevman_app* thisapp = usbdevman_app::get_instance();
	thisapp->set_verbose(1);
	devicemanager* pasmgr = new devicemanager();
	pasmgr->refresh_devices();
	if (usbdevman_app::get_instance()->is_verbose() ){
		std::cout << "dumping devices... " << std::endl;
	}
	pasmgr->dump_devices();
	delete(pasmgr);
	return 0;
}
