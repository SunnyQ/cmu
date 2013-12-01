package cmu.edu.ds.a2.test;

import java.io.IOException;
import java.lang.reflect.UndeclaredThrowableException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

import cmu.edu.ds.a2.rmi.RMINaming;
import cmu.edu.ds.a2.rmi.RemoteObjectRef;

/**
 * This serves as the driver file for the test cases
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class HelloWorldClient {
  public static void main(String args[]) throws UnknownHostException, IOException,
          ClassNotFoundException, InstantiationException, IllegalAccessException {
    /* Test 1: normal case with arguments */
    RemoteObjectRef ror = RMINaming.lookup("localhost", 4000, 3000, "HelloWorld");
    HelloWorld hello = (HelloWorld) ror.localise();
    System.out.println(hello.sayHi("Kobe"));

    /* Test 2: normal case without arguments */
    ror = RMINaming.lookup("localhost", 4000, 3000, "HelloWorldNoArgs");
    HelloWorldNoArgs helloNoArgs = (HelloWorldNoArgs) ror.localise();
    System.out.println(helloNoArgs.sayHi());

    /* Test 3: inconsistent call, more details are described in the report */
    for (int i = 0; i < 20; i++) {
      try {
        ror = RMINaming.lookup("localhost", 4000, 3000, "HelloInconsistency");
        hello = (HelloWorld) ror.localise();
        System.out.println(hello.sayHi("Kobe"));
      } catch (UndeclaredThrowableException e) {
        if (e.getCause() instanceof SocketTimeoutException) {
          System.out.println("Timeout happens - Normal condition: " + e.getCause());
          continue;
        }
      }
      System.out.println("Local counter is " + i);
    }

    /* Test 4: Timeout test, proxy server robust test */
    try {
      ror = RMINaming.lookup("localhost", 4000, 3000, "HelloTimeout");
      hello = (HelloWorld) ror.localise();
      System.out.println(hello.sayHi("Kobe"));
    } catch (UndeclaredThrowableException e) {
      if (e.getCause() instanceof SocketTimeoutException)
        System.out.println("test passed");
    }
  }
}
