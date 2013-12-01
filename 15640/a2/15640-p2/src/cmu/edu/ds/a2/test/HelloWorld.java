package cmu.edu.ds.a2.test;

import cmu.edu.ds.a2.rmi.RemoteObject;

public interface HelloWorld extends RemoteObject {
  public String sayHi(String name);
}
