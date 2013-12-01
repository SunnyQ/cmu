package cmu.edu.ds.a2.test;

public class HelloTimeoutImpl implements HelloWorld {
  @Override
  public String sayHi(String name) {
	  try {
		Thread.sleep(10000);
	} catch (InterruptedException e) {
		e.printStackTrace();
	}
    return "Hi! " + name;
  }
}
