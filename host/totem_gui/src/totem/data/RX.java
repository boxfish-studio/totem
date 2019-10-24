package totem.data;

import java.util.Observable;

public class RX extends Observable {

	private Integer foo, bar;

	public RX() {
		this.foo = null;
		this.bar = null;
	}

	public void set(int foo, int bar) {

		synchronized (this) {

			this.foo = foo;
			this.bar = bar;

		}
		setChanged();
		notifyObservers();
	}

	public Integer get_foo() {
		return foo;
	}

	public Integer get_bar() {
		return bar;
	}

	public void print() {
		System.out.println(String.format("[TOTEM RX] Foo: %d	Bar: %d", get_foo(), get_bar()));
	}
}
