package cmu.edu.ds.a2.rmi;

import java.io.IOException;
import java.net.UnknownHostException;

/**
 * This serves as an assistant for client program to transparently communicate with registry server
 * and retrieve the RemoteObjectReference information.
 * 
 * @author Yuan Gu, Yang Sun
 * 
 */
public class RMINaming {

  /**
   * Send a request to the registry server to look up the class name and collect response of a
   * remote object reference
   * 
   * @param remoteIp
   *          Registry's IP address
   * @param remotePort
   *          Registry's port number
   * @param timeout
   *          Timeout limit
   * @param serviceName
   *          Service name the client is calling with
   * @return the remote object reference
   * @throws UnknownHostException
   * @throws IOException
   * @throws ClassNotFoundException
   */
  public static RemoteObjectRef lookup(String remoteIp, int remotePort, int timeout,
          String serviceName) throws UnknownHostException, IOException, ClassNotFoundException {
    RMINamingPayload payload = new RMINamingPayload(serviceName);
    RMIMessage requestMsg = new RMIMessage(RMIMessage.TYPE.RMI_NAMING, payload);
    RMIMessage responseMsg = RMIMessage.sendRequest(remoteIp, remotePort, timeout, requestMsg);

    if (responseMsg == null || responseMsg.getType() != RMIMessage.TYPE.RMI_NAMING
            || responseMsg.getPayload() == null) {
      return null;
    }

    return ((RMINamingPayload) responseMsg.getPayload()).getRor();
  }
}
