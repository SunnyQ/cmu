package cmu.edu.ds.a2.proxy;

import cmu.edu.ds.a2.rmi.RMIMessage.RMIMessagePayload;

/**
 * This class serves as the RMIMessagePayload for communications between client stub and the server
 * proxy dispatcher. It conveys the function call information issued from the client.
 * 
 * @author Yuan Gu, Yang Sun
 * 
 */
public class RMIProxyPayload extends RMIMessagePayload {

  private static final long serialVersionUID = 1L;

  private String method;

  private String obj_key;

  private Object[] args;

  private Class<?>[] argTypes;

  private Object returnVal;

  private Exception exception;

  public RMIProxyPayload(String method, String obj_key, Object[] args, Class<?>[] argTypes) {
    this.method = method;
    this.obj_key = obj_key;
    this.args = args;
    this.argTypes = argTypes;
  }

  public String getMethod() {
    return method;
  }

  public void setMethod(String method) {
    this.method = method;
  }

  public String getObj_key() {
    return obj_key;
  }

  public void setObj_key(String obj_key) {
    this.obj_key = obj_key;
  }

  public Object[] getArgs() {
    return args;
  }

  public void setArgs(Object[] args) {
    this.args = args;
  }

  public Object getReturnVal() {
    return returnVal;
  }

  public void setReturnVal(Object returnVal) {
    this.returnVal = returnVal;
  }

  public Exception getException() {
    return exception;
  }

  public void setException(Exception exception) {
    this.exception = exception;
  }

  public Class<?>[] getArgTypes() {
    return argTypes;
  }

  public void setArgTypes(Class<?>[] argTypes) {
    this.argTypes = argTypes;
  }

}
