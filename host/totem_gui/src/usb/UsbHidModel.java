/**
 *
 * UsbHidModel.java
 * Created on: Oct 21, 2019
 *
 * Implements necessary USB HID functionality to communicate with the device
 */

package usb;

import java.io.IOException;
import java.util.List;

import com.sun.jna.Platform;

import usb.ConnectionNotifier;
import usb.hid4java.HidDevice;
import usb.hid4java.HidException;
import usb.hid4java.HidManager;
import usb.hid4java.HidServices;
import usb.hid4java.HidServicesListener;
import usb.hid4java.event.HidServicesEvent;

public class UsbHidModel  implements HidServicesListener {
	
	private int readTimeout = 250;

	private HidDevice device = null;
	static boolean hidapiInitialized = false;
	private HidServices hidServices = null;
	private ConnectionNotifier notifier = null;
	
	private int vendorID;
	private int sendArrLength;
	
	public void setDeviceVendorID(int vendorID){
		this.vendorID = vendorID;
	}
	
	public void setDeviceSendArrLength(int sendArrLength){
		this.sendArrLength = sendArrLength;
	}

	public void setListener(HidServicesListener notifier)
	{
		hidServices.addHidServicesListener(notifier);
	}

	public void setNotifier(ConnectionNotifier notifier)
	{
		this.notifier = notifier;
	}
	
	// timeout to update xmodem queues and read the device 
	// (ms)
	public void setReadTimeout(int readTimeout) {
		this.readTimeout = readTimeout;
	}
	
	public int getReadTimeout() {
		return this.readTimeout;
	}

	public void initialize()
	{
		// Get HID services
		try {
			hidServices = HidManager.getHidServices();
		} catch (HidException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		hidServices.addHidServicesListener(this);
		hidapiInitialized = true;		
	}

	public boolean deviceIsInitialized()
	{
		return hidapiInitialized;
	}
	
	public HidDevice getDevice(){
		return device;
	}
	
	public void findDeviceAttached(){
		// fire deviceAttached notifier for each connected device
	    for (HidDevice hidDevice : hidServices.getAttachedHidDevices()) {
	    	notifier.deviceAttached(hidDevice);
	    }
	}

	public boolean deviceConnect(int productID)
	{

		if (device != null)
			deviceDisconnect();

		device = hidServices.getHidDevice(vendorID, productID, null);

		if (device != null)
		{
			System.out.println("CONNECTED");
			return true;
		}
		else
		{
			System.err.println("Device not found!");
			return false;
		}
	}

	public boolean deviceDisconnect()
	{
		//System.err.println("DISCONECTING " + device.getProduct());
		if (device == null)
			return true;

		device.close();
		device = null;

		return true;
	}

	public byte[] deviceRead() throws IOException
	{
		//TODO: Set real maximum size (XMODEM)
		final byte[] data = new byte[200];
		int read = 0;
		
		try{
			read = device.read(data, readTimeout);
		}
		catch(Exception e){
			read = -1;
		}

		/* Error occurred - nothing read */
		if (read < 0)
			throw new IOException("Cannot read from USB device!");

		byte[] ret_val = new byte[read];
		for (int i = 0; i < read; i++)
			ret_val[i] = data[i];

		return ret_val;
	}

	public boolean deviceWrite(byte[] byteArr) {

		byte sendArr[] = new byte[sendArrLength];

		try{
			if (Platform.isWindows())
			{
				sendArr[0] = (byte)0x00;
				System.arraycopy(byteArr, 0, sendArr, 1, byteArr.length);
				device.write(sendArr, sendArrLength, (byte)0x00);
			}
			else
			{
				System.arraycopy(byteArr, 0, sendArr, 0, byteArr.length);
				device.write(sendArr, sendArrLength - 1, (byte)0x00);
			}
		}
		catch(Exception e){
			return false;
		}
		

		return true;
	}
	
	@Override
	public void hidDeviceAttached(HidServicesEvent event)
	{
		System.out.println("New device attached! (" + event.getHidDevice().toString() + ")");
		if (notifier != null && event.getHidDevice().getVendorId() == vendorID)
			notifier.deviceAttached(event.getHidDevice());
	}

	@Override
	public void hidDeviceDetached(HidServicesEvent event) {
		System.out.println("Device Detached! (" + event.getHidDevice().toString() + ")");

		if (notifier != null && event.getHidDevice().getVendorId() == vendorID)
			notifier.deviceDetached(event.getHidDevice());
	}

	@Override
	public void hidFailure(HidServicesEvent event) {
		System.err.println("Device failure! (" + event.getHidDevice().toString() + ")");
	}
}
