/*
 *	Java Usb Device Management
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

package jpmp.device;

import jpmp.notifier.IDeviceTransferNotifier;
import jpmp.notifier.IParseTreeNotifier;

/**
 * USB device implementation
 * 
 * @author cmi
 * <p>Implement an USB devices</p>
 * <p>
 * USB devices can be connected as MSC or MTP
 * </p>
 *
 */
public class UsbDevice {
	
	/**
	 * USB device connection type : USB connected, but not MSC / MTP
	 */
	private static final int USB_MODE_CONNECTED = 0;
	/**
	 * USB device connection type : USB connected MSC
	 */
	private static final int USB_MODE_MSC = 1;
	/**
	 * USB device connection type : USB connected MTP
	 */
	private static final int USB_MODE_MTP = 2;
	
	/**
	 * Success (native method returns with no error)
	 */
	public static final long ERROR_SUCCESS 				= 0;
	/**
	 * Unknown device
	 * <p>This can happen when the USB device has been disconnected</p>
	 */
	public static final long ERROR_UNKNOWN_DEVICE 		= 1;
	/**
	 * File not found on the USB device
	 */
	public static final long ERROR_FILE_NOTFOUND 		= 2;
	/**
	 * Folder not found on the USB device
	 */
	public static final long ERROR_FOLDER_NOTFOUND 		= 3;
	/**
	 * Folder not empty on the USB device
	 */
	public static final long ERROR_FOLDER_NOTEMPTY 		= 4;
	/**
	 * Error on metadata property
	 */
	public static final long ERROR_METADATA_PROPERTY 	= 5;
	/**
	 * Jni error on invoking object method
	 */
	public static final long ERROR_INVOKE_METHOD 		= 6;
	/**
	 * Error initializing native device manager
	 */
	public static final long ERROR_INIT_DEViCEMANAGER	= 7;
	/**
	 * The USB device is not mounted (not MSC or MTP)
	 */
	public static final long ERROR_DEVICE_NOTMOUNTED	= 8;
	/**
	 * Path exists
	 */
	public static final long ERROR_PATH_EXISTS			= 9;
	/**
	 * Not available for MTP connected USB device
	 */
	public static final long ERROR_MTP_NOTAVAILABLE 	= 10;
	/**
	 * Not available for MSC connected USB device
	 */
	public static final long ERROR_MSC_NOTAVAILABLE 	= 11;
	/**
	 * Incorrect parameter
	 */
	public static final long ERROR_WRONG_PARAMETER 		= 12;
	/**
	 * Unknown error
	 */
	public static final long ERROR_UNKNOWN 				= 1000;
	/**
	 * Not implemented on native side
	 */
	public static final long ERROR_NOTIMPL 				= 2000;
	
	private String canonical;
	private String name;
	private String serial;
	private String pid;
	private String vid;
	private String mountPoint;
	private String manufacturer;
	private String videoFolder;
	private String audioFolder;
	private String pictureFolder;
	private int usbIndex;
	private long totalSize;
	private long freeSize;
	private int productid;
	private int vendorid;
	private int usbMode = USB_MODE_CONNECTED;
	
	/**
	 * Getter for USB device canonical string
	 * @return USB device canonical string
	 */
	public String getCanonical() {
		return canonical;
	}
	/**
	 * Setter for USB device canonical string
	 * @param canonical USB device canonical string
	 */
	public void setCanonical(String canonical) {
		this.canonical = canonical;
	}
	/**
	 * Getter for USB device name
	 * @return USB device name
	 */
	public String getName() {
		return name;
	}
	/**
	 * Setter for USB device name
	 * @param name USB device name
	 */
	public void setName(String name) {
		this.name = name;
	}
	/**
	 * Getter for USB device serial number
	 * @return USB device serial number
	 */
	public String getSerial() {
		return serial;
	}
	/**
	 * Setter for USB device serial number
	 * @param serial USB device serial number
	 */
	public void setSerial(String serial) {
		this.serial = serial;
	}
	/**
	 * Getter for USB device product ID string
	 * @return USB device product ID string
	 */
	public String getPid() {
		return pid;
	}
	/**
	 * Setter for USB device product ID string
	 * @param pid USB device product ID string
	 */
	public void setPid(String pid) {
		this.pid = pid;
	}
	/**
	 * Getter for USB device vendor ID string
	 * @return USB device vendor ID string
	 */
	public String getVid() {
		return vid;
	}
	/**
	 * Setter for USB device vendor ID string
	 * @param vid USB device vendor ID string
	 */
	public void setVid(String vid) {
		this.vid = vid;
	}
	/**
	 * Getter for USB device mount point, only for MSC
	 * @return USB device mount point, only for MSC
	 */
	public String getMountPoint() {
		return mountPoint;
	}
	/**
	 * Setter for USB device mount point, only for MSC
	 * @param mountPoint USB device mount point, only for MSC
	 */
	public void setMountPoint(String mountPoint) {
		this.mountPoint = mountPoint;
	}
	/**
	 * Getter for USB device manufacturer
	 * @return USB device manufacturer
	 */
	public String getManufacturer() {
		return manufacturer;
	}
	/**
	 * Setter for USB device manufacturer
	 * @param manufacturer USB device manufacturer
	 */
	public void setManufacturer(String manufacturer) {
		this.manufacturer = manufacturer;
	}
	/**
	 * Getter for USB device video folder
	 * @return USB device video folder
	 */
	public String getVideoFolder() {
		return videoFolder;
	}
	/**
	 * Setter for USB device video folder
	 * @param videoFolder USB device video folder
	 */
	public void setVideoFolder(String videoFolder) {
		this.videoFolder = videoFolder;
	}
	/**
	 * Getter for USB device audio folder
	 * @return USB device audio folder
	 */
	public String getAudioFolder() {
		return audioFolder;
	}
	/**
	 * Setter for USB device audio folder
	 * @param audioFolder USB device audio folder
	 */
	public void setAudioFolder(String audioFolder) {
		this.audioFolder = audioFolder;
	}
	/**
	 * Getter for USB device picture folder
	 * @return USB device picture folder
	 */
	public String getPictureFolder() {
		return pictureFolder;
	}
	/**
	 * Setter for USB device picture folder
	 * @param pictureFolder USB device picture folder
	 */
	public void setPictureFolder(String pictureFolder) {
		this.pictureFolder = pictureFolder;
	}
	/**
	 * Getter for USB device index
	 * @return USB device index
	 */
	public int getUsbIndex() {
		return usbIndex;
	}
	/**
	 * Setter for USB device index
	 * @param usbIndex USB device index
	 */
	public void setUsbIndex(int usbIndex) {
		this.usbIndex = usbIndex;
	}
	/**
	 * Getter for USB device total space
	 * @return USB device total space
	 */
	public long getTotalSize() {
		return totalSize;
	}
	/**
	 * Setter for USB device total space
	 * @param totalSize USB device total space
	 */
	public void setTotalSize(long totalSize) {
		this.totalSize = totalSize;
	}
	/**
	 * Getter for USB device free space
	 * @return USB device free space
	 */
	public long getFreeSize() {
		return freeSize;
	}
	/**
	 * Setter for USB device free space
	 * @param freeSize USB device free space
	 */
	public void setFreeSize(long freeSize) {
		this.freeSize = freeSize;
	}
	/**
	 * Getter for USB device product ID
	 * @return USB device product ID
	 */
	public int getProductid() {
		return productid;
	}
	/**
	 * Setter for USB device product ID
	 * @param productid USB device product ID
	 */
	public void setProductid(int productid) {
		this.productid = productid;
	}
	/**
	 * Getter for USB device vendor ID
	 * @return USB device vendor ID
	 */
	public int getVendorid() {
		return vendorid;
	}
	/**
	 * Setter for USB device vendor ID
	 * @param vendorid USB device vendor ID
	 */
	public void setVendorid(int vendorid) {
		this.vendorid = vendorid;
	}
	/**
	 * Getter for USB device mode - MSC/MTP
	 * @return USB device mode - MSC/MTP
	 */
	public int getUsbMode() {
		return usbMode;
	}
	/**
	 * Setter for USB device mode - MSC/MTP
	 * @param usbMode USB device mode - MSC/MTP
	 */
	public void setUsbMode(int usbMode) {
		this.usbMode = usbMode;
	}
	/**
	 * Set the device mode as MSC
	 */
	public void setMscMode(){
		usbMode = USB_MODE_MSC;
	}
	/**
	 * Set the device mode as MTP
	 */
	public void setMtpMode(){
		usbMode = USB_MODE_MTP;
	}
	/**
	 * Retrieve a string used as device identification
	 * when checking device presence on USB port
	 * @return a string used as device identification
	 */
	public final String getDeviceIdentifier(){
		String result = String.format("%s_%s_%04x_%04x", new Object[]{name, serial, new Integer(vendorid), new Integer(productid)});
		return result;
	}
	/**
	 * Dump the device information, for debugging
	 * @return device information as String
	 */
	public String dump(){
		StringBuffer sb = new StringBuffer();
		sb.append("[serial = " + serial);
		sb.append("; pid = " + pid);
		sb.append("; vid = " + vid);
		sb.append("; name = " + name);
		sb.append("; manufacturer = " + manufacturer);
		sb.append("; mount point = " + mountPoint);
		sb.append("; video folder = " + videoFolder);
		sb.append("; audio folder = " + audioFolder);
		sb.append("; picture folder = " + pictureFolder);
		sb.append("; index = " + usbIndex);
		sb.append("; capacity = " + totalSize);
		sb.append("; canonical = " + canonical + "]");
		return sb.toString();
	}
	
	/**
	 * Check file existence on the USB device, native.
	 * @param pmpFilePath file to check
	 * @return 0 if success
	 */
	public native long	existFile(String pmpFilePath);
	/**
	 * Delete file from the USB device, native.
	 * @param pmpFilePath file to delete
	 * @return 0 if success
	 */
	public native long 	deleteFile(String pmpFilePath);
	/**
	 * Check folder existence on the USB device, native.
	 * @param pmpFilePath folder to check
	 * @return 0 if success
	 */
	public native long 	existFolder(String pmpFilePath);
	/**
	 * Delete folder from the USB device, native.
	 * @param pmpFolderPath folder to delete
	 * @return 0 if success
	 */
	public native long 	deleteFolder(String pmpFolderPath);
	/**
	 * Create folder on the USB device, native.
	 * @param pmpFolderPath folder to create
	 * @return 0 if success
	 */
	public native long 	createFolder(String pmpFolderPath);
	/**
	 * Get metadata information for a file, only MTP. For MSC this method does nothing.
	 * @param pmpSource file to retrieved metadata for
	 * @param metadata metadata information retrieved
	 * @return 0 if success
	 */
	public native long 	getMetadata(String pmpSource, FileMetadata metadata );
	/**
	 * Update metadata information for a file, only MTP. For MSC this method does nothing.
	 * @param pmpSource file to update metadata
	 * @param metadata metadata information to update, only MTP.
	 * @return 0 if success
	 */
	public native long 	updateMetadata(String pmpSource, FileMetadata metadata );
	/**
	 * Get a file from the USB device to local file system
	 * @param localDest local path destination (if the destination exists, it will be replaced) 
	 * @param pmpSource file to get from the USB device
	 * @param notif transfer progress notifier
	 * @return 0 if success
	 */
	public native long 	getFile(String localDest, String pmpSource, IDeviceTransferNotifier notif);
	/**
	 * Send a file to the USB device
	 * @param localSource local path source file
	 * @param pmpDestination destination path on the USB device (if the destination exists, it will be replaced) 
	 * @param notif transfer progress notifier
	 * @param metadata metadata information to associate with the file, only MTP. For MSC this is ignored.
	 * @return 0 if success
	 */
	public native long 	sendFile(String localSource, String pmpDestination, IDeviceTransferNotifier notif, FileMetadata metadata);
	/**
	 * Parse folder on the USB device
	 * @param pmpDestination path to parse on the USB device 
	 * @param notif callback to get files and folders
	 * @return 0 if success
	 */
	public native long 	parseFolder(String pmpDestination, IParseTreeNotifier notif);
}
