package cmu.edu.ds.a2.rmi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

/**
 * The Class RMIMessage implements a generic message for RMI communication. A RMIMessage consists of
 * two parts: the type and the payload. The type defines what kind of payloads this RMIMessage
 * carries and the payload is an object carrying the message body. All payload should extends the
 * RMIMessagePayload class.
 * 
 * @author Yuan Gu, Yang Sun
 * 
 */
public class RMIMessage implements Serializable {

  public static enum TYPE {
    RMI_PROXY, RMI_REGISTRY, RMI_NAMING,
  }

  public static abstract class RMIMessagePayload implements Serializable {
    private static final long serialVersionUID = 7578843532696391385L;
  }

  private static final long serialVersionUID = 3085890411891643458L;

  private TYPE type;

  private RMIMessagePayload payload;

  /**
   * Send a RMIMessage to a host and return the response RMIMessage
   * 
   * @param remoteIp
   *          Server's IP address
   * @param remotePort
   *          Server's port number
   * @param timeout
   *          Timeout limit
   * @param requestMsg
   *          RMIMessage being sent
   * @return the response RMIMessage
   * @throws UnknownHostException
   * @throws IOException
   * @throws ClassNotFoundException
   */
  public static RMIMessage sendRequest(String remoteIp, int remotePort, int timeout,
          RMIMessage requestMsg) throws UnknownHostException, IOException, ClassNotFoundException,
          SocketTimeoutException {
    Socket sock = new Socket(remoteIp, remotePort);
    sock.setSoTimeout(timeout);

    /* send request RMIMessage to remote */
    ObjectOutputStream out = new ObjectOutputStream(sock.getOutputStream());
    out.writeObject(requestMsg);
    out.flush();

    /* keep waiting for incoming messages */
    ObjectInputStream in = new ObjectInputStream(sock.getInputStream());
    RMIMessage responseMsg = (RMIMessage) in.readObject();

    out.close();
    in.close();
    sock.close();

    return responseMsg;

  }

  public RMIMessage(TYPE rmiProxy, RMIMessagePayload payload) {
    this.type = rmiProxy;
    this.payload = payload;
  }

  public TYPE getType() {
    return type;
  }

  public void setType(TYPE type) {
    this.type = type;
  }

  public RMIMessagePayload getPayload() {
    return payload;
  }

  public void setPayload(RMIMessagePayload payload) {
    this.payload = payload;
  }
}
