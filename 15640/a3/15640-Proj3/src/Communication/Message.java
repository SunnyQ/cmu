package Communication;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

/**
 * Class: Message.java
 * 
 * The Message passed back and forth between master and clients
 * 
 * @author Yuan Gu
 * 
 */
public class Message implements Serializable {

  public static enum TYPE {
    REGISTER, SUBMIT, START, STOP, MONITOR, STATUS, MAPPER, REDUCER, RESULT, EXCEPTION,
  }

  public static abstract class MessagePayload implements Serializable {
    private static final long serialVersionUID = 7578843532696391385L;
  }

  private static final long serialVersionUID = 3085890411891643458L;

  private TYPE type;

  private MessagePayload payload;

  /**
   * Send a Message to a host and return the response Message
   * 
   * @param remoteIp
   *          Server's IP address
   * @param remotePort
   *          Server's port number
   * @param timeout
   *          Timeout limit
   * @param requestMsg
   *          Message being sent
   * @return the response Message
   * @throws UnknownHostException
   * @throws IOException
   * @throws ClassNotFoundException
   */
  public static Message sendRequest(String remoteIp, int remotePort, int timeout, Message requestMsg)
          throws UnknownHostException, IOException, ClassNotFoundException, SocketTimeoutException {
    Socket sock = new Socket(remoteIp, remotePort);
    sock.setSoTimeout(timeout);

    /* send request RMIMessage to remote */
    ObjectOutputStream out = new ObjectOutputStream(sock.getOutputStream());
    out.writeObject(requestMsg);
    out.flush();

    /* keep waiting for incoming messages */
    ObjectInputStream in = new ObjectInputStream(sock.getInputStream());
    Message responseMsg = (Message) in.readObject();

    out.close();
    in.close();
    sock.close();

    return responseMsg;

  }

  public Message(TYPE type, MessagePayload payload) {
    this.type = type;
    this.payload = payload;
  }

  public TYPE getType() {
    return type;
  }

  public void setType(TYPE type) {
    this.type = type;
  }

  public MessagePayload getPayload() {
    return payload;
  }

  public void setPayload(MessagePayload payload) {
    this.payload = payload;
  }
}
