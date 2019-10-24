package totem;

import java.util.Arrays;
import java.util.Observable;

import totem.data.Data;
import totem.xmodem_service.TotemDeviceConfig;
import totem.xmodem_service.XMODEMService;
import usb.ConnectionNotifier;
import usb.UsbHidModel;
import usb.hid4java.HidDevice;

public class Totem extends Observable implements Runnable {

	private UsbHidModel usbHidModel;
	private USBStatus usb_status;
	private ConnectionNotifier connection_status;

	private XMODEMService xmodem_bridge;
	private Thread device_sync_thread;
	private boolean xmodem_sync_enable;

	private Data totem_data;

	public class USBStatus extends Observable {
		private volatile boolean connected;

		public USBStatus() {
			this.connected = false;
		}

		public boolean is_connected() {
			return connected;
		}

		public void set_connected(boolean connected) {
			synchronized (Totem.this) {
				this.connected = connected;
			}
			setChanged();
			notifyObservers();
		}
	}

	public Totem() {

		this.totem_data = new Data();
		this.usb_status = new USBStatus();

		initUSB();

		device_sync_thread = new Thread(this);
		device_sync_thread.start();

		this.xmodem_sync_enable = true;
	}

	public void initUSB() {

		this.usbHidModel = new UsbHidModel();
		this.xmodem_bridge = new XMODEMService(this);
		this.connection_status = new ConnectionNotifier() {

			@Override
			public void deviceAttached(HidDevice device) {

				if (device.getProductId() == TotemDeviceConfig.PRODUCT_ID) {
					System.out.println(String.format("%s DETECTED", device.getProduct()));
					if (start_usb()) {
						get_usb_status().set_connected(true);
					}
				}
			}

			@Override
			public void deviceDetached(HidDevice device) {
				if (device.getProductId() == TotemDeviceConfig.PRODUCT_ID) {
					stop_usb();
					get_usb_status().set_connected(false);
				}
			}

			@Override
			public void connectionLost() {
				// TODO Auto-generated method stub
			}

		};

		usbHidModel.initialize();
		usbHidModel.setNotifier(connection_status);
		usbHidModel.setDeviceVendorID(TotemDeviceConfig.VENDOR_ID);
		usbHidModel.setDeviceSendArrLength(TotemDeviceConfig.SENDARR_LENGHT);
		usbHidModel.setReadTimeout(TotemDeviceConfig.READ_TIMEOUT);
	}

	public UsbHidModel getUsbHidModel() {
		return this.usbHidModel;
	}

	public USBStatus get_usb_status() {
		return this.usb_status;
	}

	public boolean start_usb() {

		// If not Product ID for regular eBike operation or Bootloader mode, don't start
		if (!usbHidModel.deviceConnect(TotemDeviceConfig.PRODUCT_ID)) {
			return false;
		} else {
			this.xmodem_bridge.init();
			return true;
		}
	}

	// Method to stop USB communication
	public void stop_usb() {
		this.xmodem_bridge.stopThread();
	}

	public Data get_eol_shield() {
		return this.totem_data;
	}

	public ConnectionNotifier get_connection_status() {
		return connection_status;
	}

	public void enable_sync(boolean set) {
		this.xmodem_sync_enable = set;
	}

	public void run() {

		System.out.println("Device Sync started");

		while (!device_sync_thread.isInterrupted()) {

			try {
				Thread.sleep(30);
			} catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
				return;
			}

			if (this.xmodem_sync_enable) {
				// Sending general tx frame
				this.xmodem_bridge.send(totem_data.xmodem_serializer_tx(totem_data.get_tx()));
			}
		}
	}

	public void dispatch_xmodem(byte data[]) {

		byte command = data[0];
		byte subcommand = data[1];

		byte[] payload = Arrays.copyOfRange(data, 2, data.length); // remove command & subcommand

		if (command == Data.Defs.XMODEM_CMD_A) {

			switch (subcommand) {

			case Data.Defs.XMODEM_SUBCMD_A:
				this.totem_data.set_rx(totem_data.xmodem_deserializer_rx(payload));
				System.out.println("dispatched subcmd A");
				break;

			default:
				System.err.println("dispatch(): Received message not valid!");
				break;
			}
		} else {
			System.err.println("dispatch(): Received message not valid!");
		}
	}
}
