package cmu.edu.ds.a2.proxy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

import cmu.edu.ds.a2.rmi.RMIMessage;

/**
 * The Class RMIProxyHandler is used as the implementation of the stub class. Once it's invoked by
 * the Java reflection mechanism, it will communication with the remote server to perform the
 * method.
 * 
 * This class is modified from ProxyDemo posted on the course website.
 * 
 * @author Yuan Gu, Yang Sun
 */

public class RMIProxyHandler implements InvocationHandler {

  private String obj_key;

  private String remoteIp;

  private int remotePort;

  private int timeout;

  public RMIProxyHandler(String remoteIp, int remotePort, int timeout, String obj_key) {
    this.obj_key = obj_key;
    this.remoteIp = remoteIp;
    this.remotePort = remotePort;
    this.timeout = timeout;
  }

  @Override
  public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {

    /* Prepare the argument types array for the proxy dispatcher to invoke the method */
    Class<?>[] argTypes = new Class<?>[args == null ? 0 : args.length];
    for (int i = 0; i < argTypes.length; i++) {
      argTypes[i] = args[i].getClass();
    }

    /* Set up the proxy payload, call the method on the remote service and collect the returns */
    RMIProxyPayload payload = new RMIProxyPayload(method.getName(), this.obj_key, args, argTypes);
    RMIMessage requestMsg = new RMIMessage(RMIMessage.TYPE.RMI_PROXY, payload);
    RMIMessage responseMsg = RMIMessage.sendRequest(remoteIp, remotePort, timeout, requestMsg);
    if (responseMsg == null || responseMsg.getType() != RMIMessage.TYPE.RMI_PROXY
            || responseMsg.getPayload() == null) {
      return null;
    }

    RMIProxyPayload responsePayload = ((RMIProxyPayload) responseMsg.getPayload());
    
    /* In case of any exception, throw it to the client */
    if (responsePayload.getException() != null)
      throw responsePayload.getException();
    return responsePayload.getReturnVal();
  }
}
