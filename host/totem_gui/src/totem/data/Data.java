package totem.data;

import java.util.Observable;

public class Data extends Observable {

	public class Defs {

		// XMODEM HOST2DEVICE COMMANDS
		public static final byte XMODEM_CMD_A = (byte) 0x20;

		// XMODEM HOST2DEVICE SUBCOMMANDS
		public static final byte XMODEM_SUBCMD_A = (byte) 0x07;

		// PAYLOAD SIZES
		public static final int XMODEM_RX_PAYLOAD_SIZE = 2;
		public static final int XMODEM_TX_PAYLOAD_SIZE = 2;

	}

	public enum Mode {
		NONE
	}

	private Mode mode;

	private RX rx;
	private TX tx;

	public Data() {
		init();
	}

	public void init() {

		this.mode = Mode.NONE;

		this.rx = new RX();
		this.tx = new TX();
	}

	public void set_rx(RX rx) {

		synchronized (this) {
			this.rx = rx;
		}
		setChanged();
		notifyObservers();

		handle_response();
	}

	public void handle_response() {

		switch (mode) {
		case NONE:
			break;

		default:
			break;
		}
	}

	public void set_mode(Mode mode) {
		this.mode = mode;
	}

	public TX get_tx() {
		return this.tx;
	}

	public RX get_rx() {
		return this.rx;
	}

	public byte[] xmodem_serializer_read_general() {

		byte[] data = new byte[2];
		data[0] = Defs.XMODEM_CMD_A;
		data[1] = Defs.XMODEM_SUBCMD_A;

		return data;
	}

	public byte[] xmodem_serializer_tx(TX tx) {

		byte[] data = new byte[Defs.XMODEM_TX_PAYLOAD_SIZE + 2];
		data[0] = Defs.XMODEM_CMD_A;

		int foo = tx.get_foo();
		data[1] = (byte) (foo & 0x000000FF);
		
		int bar = tx.get_bar();
		data[2] = (byte) (bar & 0x000000FF);

		return data;
	}

	public RX xmodem_deserializer_rx(byte[] data) {

		int foo = (data[0] & 0x000000FF);
		int bar = (data[1] & 0x000000FF);

		RX rx = new RX();
		rx.set(foo, bar);

		return rx;
	}

	public void print() {
		this.rx.print();
		this.tx.print();
	}
}
