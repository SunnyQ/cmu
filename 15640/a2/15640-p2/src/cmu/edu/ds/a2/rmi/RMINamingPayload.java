package cmu.edu.ds.a2.rmi;

import cmu.edu.ds.a2.rmi.RMIMessage.RMIMessagePayload;

/**
 * This serves as the payload sector for the communication between the client and the registry
 * server
 * 
 * @author Yuan Gu, Yang Sun
 * 
 */
public class RMINamingPayload extends RMIMessagePayload {

  private static final long serialVersionUID = -4618453128143277161L;

  private String serviceName;

  private RemoteObjectRef ror;

  public RMINamingPayload(String serviceName) {
    this.serviceName = serviceName;
  }

  public String getServiceName() {
    return serviceName;
  }

  public void setServiceName(String serviceName) {
    this.serviceName = serviceName;
  }

  public RemoteObjectRef getRor() {
    return ror;
  }

  public void setRor(RemoteObjectRef ror) {
    this.ror = ror;
  }

}
