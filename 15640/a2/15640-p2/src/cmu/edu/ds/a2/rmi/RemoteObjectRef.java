package cmu.edu.ds.a2.rmi;

import java.lang.reflect.Proxy;

import cmu.edu.ds.a2.proxy.RMIProxyHandler;
import cmu.edu.ds.a2.rmi.RMIMessage.RMIMessagePayload;

/**
 * This class serves as the remote object representation transferred between client and server.It
 * contains the information the client that the client needs in order to locate the corresponding
 * object running on the server.
 * 
 * This class is modified from the version posted on the course website.
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class RemoteObjectRef extends RMIMessagePayload {
  private static final long serialVersionUID = -7628838141171095001L;

  private String IP_adr;

  private int Port;

  private String Obj_Key;

  private String Remote_Interface_Name;

  public RemoteObjectRef(String ip, int port, String obj_key, String riname) {
    setIP_adr(ip);
    setPort(port);
    setObj_Key(obj_key);
    setRemote_Interface_Name(riname);
  }

  /**
   * Create a proxy for the client used for communicating with the server.
   * 
   * @return the object interface
   * @throws ClassNotFoundException
   * @throws InstantiationException
   * @throws IllegalAccessException
   */
  public Object localise() throws ClassNotFoundException, InstantiationException,
          IllegalAccessException {

    RMIProxyHandler handler = new RMIProxyHandler(this.IP_adr, this.Port, 3000, this.Obj_Key);
    Class<?> ifClass = Class.forName(this.Remote_Interface_Name);
    Object proxy = Proxy.newProxyInstance(ifClass.getClassLoader(), new Class[] { ifClass },
            handler);

    return proxy;
  }

  public String getIP_adr() {
    return IP_adr;
  }

  public void setIP_adr(String iP_adr) {
    IP_adr = iP_adr;
  }

  public int getPort() {
    return Port;
  }

  public void setPort(int port) {
    Port = port;
  }

  public String getObj_Key() {
    return Obj_Key;
  }

  public void setObj_Key(String obj_Key) {
    Obj_Key = obj_Key;
  }

  public String getRemote_Interface_Name() {
    return Remote_Interface_Name;
  }

  public void setRemote_Interface_Name(String remote_Interface_Name) {
    Remote_Interface_Name = remote_Interface_Name;
  }
}
