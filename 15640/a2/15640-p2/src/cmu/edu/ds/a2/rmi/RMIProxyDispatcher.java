package cmu.edu.ds.a2.rmi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import cmu.edu.ds.a2.proxy.RMIProxyPayload;

/**
 * This serves as the proxy dispatcher standing on the server side. It registers remote objects
 * available with the registry server and receives the function calls from the client.
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class RMIProxyDispatcher {

  /**
   * This serves as the RMIService receiving the function call from the client and return the result
   * back
   * 
   * @author Yang Sun, Yuan Gu
   * 
   */
  public class RMIService implements Runnable {
    private Socket sock;

    private RMIMessage incomMessage;

    public RMIService(RMIMessage incomingMsg, Socket sock) {
      this.sock = sock;
      this.incomMessage = incomingMsg;
    }

    @Override
    public void run() {
      RMIProxyPayload payload = (RMIProxyPayload) incomMessage.getPayload();
      try {
        /* fetch the target method */
        RemoteObject obj = pool.get(payload.getObj_key());
        Method method = obj.getClass().getMethod(payload.getMethod(), payload.getArgTypes());

        try {
          /* on success call, set the return value */
          payload.setReturnVal(method.invoke(pool.get(payload.getObj_key()), payload.getArgs()));
        } catch (Exception e) {
          /* on failure, set the exception */
          payload.setException(e);
        }
      } catch (SecurityException e) {
        payload.setException(e);
      } catch (NoSuchMethodException e) {
        payload.setException(e);
      } catch (IllegalArgumentException e) {
        payload.setException(e);
      } finally {

        /* write the RMIMessage back after collection the result from the local function call */
        try {
          ObjectOutputStream out = new ObjectOutputStream(sock.getOutputStream());
          out.writeObject(incomMessage);
          out.flush();
          out.close();

          if (!sock.isClosed())
            sock.close();

        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
  }

  private static String registryAddr;

  private static int serverPort;

  private static int registryPort;

  private Map<String, RemoteObject> pool;

  public RMIProxyDispatcher() throws IOException {
    pool = new HashMap<String, RemoteObject>();
  }

  /**
   * Communicate with the registry server and register the remote object on the registry
   * 
   * @param ro
   *          the remote object pending to be registered
   * @throws IOException
   */
  public void exportRemoteService(RemoteObject ro, String serviceName) throws IOException {
    pool.put(serviceName, ro);

    /* prepare the RemoteObjectRef and RMIMessage */
    RemoteObjectRef ror = new RemoteObjectRef(null, serverPort, serviceName, ro.getClass()
            .getInterfaces()[0].getName());
    RMIMessage msg = new RMIMessage(RMIMessage.TYPE.RMI_REGISTRY, ror);

    /* Write to registry server */
    Socket registrySock = new Socket(registryAddr, registryPort);
    ObjectOutputStream outStream = new ObjectOutputStream(registrySock.getOutputStream());
    outStream.writeObject(msg);
    outStream.flush();

    outStream.close();
    registrySock.close();
  }

  /**
   * Run the proxy dispatcher as a service and receives the function call from the client
   */
  public void serve() {
    try {
      ServerSocket serverSock = new ServerSocket(serverPort);
      Executor executor = Executors.newCachedThreadPool();
      for (;;) {
        Socket sock = serverSock.accept();
        ObjectInputStream in = new ObjectInputStream(sock.getInputStream());
        RMIMessage incomingMsg = (RMIMessage) in.readObject();
        if (incomingMsg == null || incomingMsg.getType() != RMIMessage.TYPE.RMI_PROXY)
          continue;

        /* Run the service concurrently */
        executor.execute(new RMIService(incomingMsg, sock));
      }
    } catch (IOException e) {
      e.printStackTrace();
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    }
  }

  public static void main(String[] args) throws IOException, InterruptedException {
    if (args.length < 8) {
      System.out
              .println("Usage: RMIProxyDispatcher -c <registryAddr> -rp <registryPort> -p <serverPort> -classes <className1:URL1;...;classNameN:URLN>");
      System.exit(0);
    }

    String strClasses = "";

    for (int i = 0; i < args.length; i++) {
      if (args[i].equals("-p"))
        serverPort = Integer.parseInt(args[++i]);
      else if (args[i].equals("-c"))
        registryAddr = InetAddress.getByName(args[++i]).getHostAddress();
      else if (args[i].equals("-rp"))
        registryPort = Integer.parseInt(args[++i]);
      else if (args[i].equals("-classes"))
        strClasses = args[++i];
    }

    String[] classes = strClasses.split(";");

    RMIProxyDispatcher dispatcher = new RMIProxyDispatcher();

    for (String strClass : classes) {
      String[] tmpArray = strClass.split(":", 2);
      for (String str : tmpArray)
        System.out.println(str);
      try {
        Class<?> c = Class.forName(tmpArray[0]);
        dispatcher.exportRemoteService((RemoteObject) c.newInstance(), tmpArray[1]);
      } catch (Exception e) {
        e.printStackTrace();
        return;
      }
    }
    dispatcher.serve();

    System.exit(1);
  }
}
