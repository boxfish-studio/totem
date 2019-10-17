package usb;

import usb.hid4java.HidDevice;

public interface ConnectionNotifier 
{
	void connectionLost();
	void deviceAttached(HidDevice device);
	void deviceDetached(HidDevice device);
}
