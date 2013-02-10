import java.util.Map.Entry;
import java.util.Set;

import jpmp.device.UsbDevice;
import jpmp.manager.DeviceManager;

import org.junit.Test;


public class DeviceManagerTest
{

    @Test
    public void testDevice() throws Throwable
    {
        DeviceManager dm = DeviceManager.getInstance();
        dm.createInstance();
        dm.scanDevices();
        
        Set<Entry<String, UsbDevice>> devices = dm.getDeviceList().entrySet();
        for(Entry<String, UsbDevice> device : devices)
        {
            UsbDevice usbdev = device.getValue();
            System.out.println(usbdev.existFile("/Music/Aalto - Resolution (Original Mix).mp3"));
            //usbdev.getFile("c:/file.mp3", "/Music/Aalto - Resolution (Original Mix).mp3", new Notifier());
            usbdev.parseFolder("/", new ParseTreeNotifier());
        }
        
    }
}
