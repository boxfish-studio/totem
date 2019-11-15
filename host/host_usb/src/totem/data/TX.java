package totem.data;

import java.util.Observable;

public class TX extends Observable {

	private Integer foo, bar;

	public TX() {

		this.foo = null;
		this.bar = null;

		init();
	}

	public void init() {

		this.foo = 0;
		this.bar = 0;
	}

	public void set(int foo, int bar) {

		this.foo = foo;
		this.bar = bar;
	}

	public Integer get_foo() {
		return foo;
	}

	public Integer get_bar() {
		return bar;
	}

	public void print() {
		System.out.println(String.format("[TOTEM TX] foo:	%d	bar: 	%d", get_foo(), get_bar()));
	}
}
