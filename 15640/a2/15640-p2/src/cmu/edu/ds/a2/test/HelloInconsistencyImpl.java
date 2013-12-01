package cmu.edu.ds.a2.test;

public class HelloInconsistencyImpl implements HelloWorld {

	private int count = 0;

	@Override
	public String sayHi(String name) {
		int random = (int) (Math.random() * 10);
		if (random % 2 == 0) {
			try {
				Thread.sleep(10000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		return "Hi! " + name + " with count = " + count++;
	}
	
}
