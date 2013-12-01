package cmu.edu.ds.a2.test;

import cmu.edu.ds.a2.rmi.RemoteObject;

public interface HelloWorldNoArgs extends RemoteObject {
  public String sayHi();
}
