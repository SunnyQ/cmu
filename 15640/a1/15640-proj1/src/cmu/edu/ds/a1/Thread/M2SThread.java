package cmu.edu.ds.a1.Thread;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * This serves as the master-to-slave thread that implements Runnable interface
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class M2SThread implements Runnable {
  /**
   * The action that is issued from master node
   */
  public enum MasterAction {
    FETCHSTATS, FETCHOBJ, OFFEROBJ
  }

  private final Socket sock;

  private int numProcesses;

  private boolean isAlive;

  private volatile MasterAction action;

  private volatile int fetchNum;

  private volatile List<String> localStock;

  public M2SThread(final Socket sock) {
    this.sock = sock;
    this.localStock = Collections.synchronizedList(new ArrayList<String>());
    this.numProcesses = 0;
    this.isAlive = true;
  }

  @Override
  public void run() {
    try {
      final DataOutputStream out = new DataOutputStream(sock.getOutputStream());
      final BufferedReader in = new BufferedReader(new InputStreamReader(sock.getInputStream()));

      synchronized (this) {
        switch (action) {

          case FETCHSTATS:
            out.writeBytes("FETCHSTATS\n");
            out.flush();

            String inMsg = in.readLine();
            if (inMsg == null)
              throw new IOException();

            String[] respArr = inMsg.split(" ");
            if (respArr[0].equals("PROCESSES"))
              numProcesses = Integer.parseInt(respArr[1]);
            break;

          case FETCHOBJ:
            out.writeBytes("FETCHOBJ " + fetchNum + "\n");
            out.flush();

            String inMsg2 = in.readLine();
            if (inMsg2 == null)
              throw new IOException();

            int exactNum = Integer.parseInt(inMsg2);
            for (int i = 0; i < exactNum; i++) {
              inMsg2 = in.readLine();
              if (inMsg2 == null)
                throw new IOException();
              localStock.add(inMsg2);
            }
            break;

          case OFFEROBJ:
            out.writeBytes("OFFEROBJ " + localStock.size() + "\n");
            out.flush();
            StringBuilder msg = new StringBuilder();
            while (localStock.size() > 0)
              msg.append(localStock.remove(0) + "\n");
            out.writeBytes(msg.toString());
            System.out.println(msg.toString());
            out.flush();
            break;

          default:
            System.out.println("How can it reach here? action is unsupported");
            break;
        }
      }
    } catch (IOException e) {
      System.out.println(sock + " has been dropped...");
      isAlive = false;
    }
  }

  protected synchronized M2SThread switchAction(final MasterAction action) {
    this.action = action;
    return this;
  }

  public synchronized int getNumProcesses() {
    return numProcesses;
  }

  public synchronized boolean isAlive() {
    return isAlive;
  }

  public synchronized void setFetchNum(int fetchNum) {
    this.fetchNum = fetchNum;
  }

  public synchronized List<String> getStocks() {
    return localStock;
  }
}
