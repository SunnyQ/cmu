package cmu.edu.ds.a1.Thread;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

import cmu.edu.ds.a1.PM.Processes;
import cmu.edu.ds.a1.PM.Slaves;

/**
 * This serves as the entry point of the Master node thread
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class MasterStarterThread implements Runnable {
  private final int serverPort;

  private volatile Slaves slaves;

  private volatile Processes processes;

  public MasterStarterThread(final int serverPort, final Processes processes) {
    this.serverPort = serverPort;
    this.slaves = new Slaves();
    this.processes = processes;
  }

  @Override
  public void run() {
    try {
      ServerSocket sock = new ServerSocket(serverPort);

      // start the main communication thread
      new Thread(new MasterCommandIssuerThread(slaves, processes)).start();

      for (;;) {
        Socket slaveSock = sock.accept();
        BufferedReader in = new BufferedReader(new InputStreamReader(slaveSock.getInputStream()));

        String msg = in.readLine();

        if (msg == null)
          continue;

        System.out.println(msg);
        // receive slave connections and add to list
        if (msg.equals("REGISTER")) {
          synchronized (slaves) {
            slaves.add(new M2SThread(slaveSock));
          }
        }

      }
    } catch (IOException e) {
      e.printStackTrace();
    }
  }
}
