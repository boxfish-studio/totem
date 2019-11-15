package totem.xmodem_service;

import java.util.concurrent.ConcurrentLinkedDeque;

import totem.Totem;
import usb.xmodem.XmodemModel;

/**
 * Service which manages the instance and interface to the XMODEM stack
 *
 */
public class XMODEMService implements Runnable {

	// Xmodem stack
	private XmodemModel xmodem = null;
	private Thread xmodem_thread = null;

	// Xmodem service
	private Thread service_thread = null;
	private volatile int service_status = 0; // 0 = ready, 1 = running, 2 = stop

	// In/out queues
	private ConcurrentLinkedDeque<byte[]> rcvQueue;
	private ConcurrentLinkedDeque<byte[]> sendQueue;

	private Totem totem;

	public XMODEMService(Totem totem) {

		this.totem = totem;

		this.rcvQueue = new ConcurrentLinkedDeque<byte[]>();
		this.sendQueue = new ConcurrentLinkedDeque<byte[]>();

		xmodem = new XmodemModel(totem.getUsbHidModel(), rcvQueue, sendQueue);
	}

	public void init() {
		resetCommunicator();
		xmodem_thread = new Thread(xmodem);
		xmodem_thread.start();

		service_thread = new Thread(this);
		service_thread.start();
	}

	public void stopThread() {
		if (xmodem != null)
			xmodem.stopThread();

		/* Thread isn't running */
		if (service_status == 0)
			return;

		service_status = 3;

		/* Wait until thread stopped */
		while (service_status != 0) {
			try {
				Thread.sleep(20);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}

	// Runnable method
	public void run() {

		if (service_status != 0) {
			System.err.println("XMODEM-SERVICE: TRIED TO START THREAD AGAIN");
			return;
		}

		System.out.println("XMODEM-SERVICE START");

		/* Set status to "running" */
		service_status = 1;

		try {
			while (service_status == 1) {

				Thread.sleep(10);
				updateRcvQueue();
			}

			/* Clear queues */
			rcvQueue.clear();
			sendQueue.clear();

			service_status = 0;
			System.out.println("XMODEM-SERVICE STOPPED");

		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	// Update Xmodem RCV Queue
	private boolean updateRcvQueue() throws InterruptedException {
		byte rcvData[] = rcvQueue.poll();

		if (rcvData != null) {
			dispatch(rcvData);
		}

		return true;
	}

	// Reset Xmodem Send Queue
	public void resetCommunicator() {
		sendQueue.clear();
	}

	public void send(byte[] data) {
		this.sendQueue.add(data);
	}

	private void dispatch(byte[] data) {
		totem.dispatch_xmodem(data);
	}
}
