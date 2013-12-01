package cmu.edu.ds.a2.rmi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;

/**
 * This serves as the registry server running standalone.
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class RMIRegistry {

  private int serverPort;

  private Map<String, RemoteObjectRef> pool;

  public RMIRegistry(int port) {
    this.serverPort = port;
    this.pool = new HashMap<String, RemoteObjectRef>();
  }

  /**
   * Start the RMIRegistry service The service receives the RemoteObjectReference from the Proxy
   * dispatcher server and answer the look up requests from the client
   */
  private void start() {
    try {
      ServerSocket serverSock = new ServerSocket(serverPort);
      for (;;) {
        try {
          Socket sock = serverSock.accept();
          ObjectInputStream in = new ObjectInputStream(sock.getInputStream());
          RMIMessage incomingMsg = (RMIMessage) in.readObject();
          if (incomingMsg == null)
            continue;

          switch (incomingMsg.getType()) {
          /* RMIMessage sent from the proxy dispatcher server */
            case RMI_REGISTRY:
              RemoteObjectRef ror = (RemoteObjectRef) incomingMsg.getPayload();
              ror.setIP_adr(sock.getInetAddress().getHostAddress());
              pool.put(ror.getObj_Key(), ror);
              break;

            /* RMIMessage sent from the client */
            case RMI_NAMING:
              RMINamingPayload namingPayload = (RMINamingPayload) incomingMsg.getPayload();
              if (pool.get(namingPayload.getServiceName()) == null) {
                return;
              }

              /* write back the answer */
              ObjectOutputStream out = new ObjectOutputStream(sock.getOutputStream());
              namingPayload.setRor(pool.get(namingPayload.getServiceName()));
              out.writeObject(incomingMsg);
              out.flush();
              out.close();
              break;
            default:
              System.out.println("What??!");
          }

          in.close();
          sock.close();

        } catch (ClassNotFoundException e) {
          continue;
        }
      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public static void main(String[] args) throws IOException, InterruptedException {
    if (args.length != 1) {
      System.out.println("Usage: RMIRegistry <serverPort>");
      System.exit(0);
    }

    System.out.println("RMIRegistry is running...");
    new RMIRegistry(Integer.parseInt(args[0])).start();
    System.exit(1);
  }
}
