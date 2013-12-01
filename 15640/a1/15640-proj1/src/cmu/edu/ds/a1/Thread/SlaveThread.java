package cmu.edu.ds.a1.Thread;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.net.Socket;
import java.net.UnknownHostException;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.SerializableWrite;
import cmu.edu.ds.a1.PM.Processes;
import cmu.edu.ds.a1.PM.ThreadWrapper;

/**
 * This serves as the slave thread that deal with commands issued from the master node
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class SlaveThread implements Runnable {

  private final int serverPort;

  private final String hostAddr;

  private Processes processes;

  public SlaveThread(String hostAddr, int serverPort, Processes processes) {
    this.serverPort = serverPort;
    this.hostAddr = hostAddr;
    this.processes = processes;
  }

  @Override
  public void run() {
    try {
      Socket master = new Socket(hostAddr, serverPort);
      DataOutputStream out = new DataOutputStream(master.getOutputStream());

      // register with the master node
      out.writeBytes("REGISTER\n");
      out.flush();

      // keep waiting for incoming messages
      for (;;)
        processIncomingMsg(out, master);

    } catch (UnknownHostException e) {
      System.out.println("Unknown host, exit...");
    } catch (IOException e) {
      System.out.println("Connection refused, exit...");
    }
    System.exit(0);
  }

  /**
   * Process all kinds of incoming message sent from the master node
   * 
   * @param out
   *          the output stream so that it can relay some message back to master
   * @param master
   *          the master node socket
   * @throws IOException
   * @throws InterruptedException
   * @throws ClassNotFoundException
   */
  private void processIncomingMsg(DataOutputStream out, Socket master) {

    try {
      BufferedReader in = new BufferedReader(new InputStreamReader(master.getInputStream()));
      String incomingMsg = in.readLine();

      synchronized (processes) {
        if (incomingMsg == null) {
          System.out.println("Connection refused, exit...");
          System.exit(0);
        }

        // reply with the number processes running on this slave
        else if (incomingMsg.equals("FETCHSTATS")) {
          out.writeBytes("PROCESSES " + processes.size() + "\n");
          out.flush();
        }

        else if (incomingMsg.startsWith("FETCHOBJ"))
          processFetchObjCall(out, Integer.parseInt(incomingMsg.split(" ")[1]));

        else if (incomingMsg.startsWith("OFFEROBJ"))
          processOfferObjCall(in, Integer.parseInt(incomingMsg.split(" ")[1]));

        else
          System.out.println("Received unsupported command: " + incomingMsg);
      }

    } catch (IOException e) {
      System.out.println("Connection refused, exit...");
      System.exit(0);
    }
  }

  /**
   * Suspend the process and transfer the address of the serialized object to the master
   * 
   * @param out
   *          the output stream to the master node
   * @param numReqs
   *          number of processes the master is requesting
   * @throws InterruptedException
   * @throws IOException
   */
  private void processFetchObjCall(final DataOutputStream out, final int numReqs) {
    // in case that processes are done while the fetching step
    try {
      int processesSize = processes.size();
      int minFetches = Math.min(numReqs, processesSize);

      out.writeBytes(minFetches + "\n");
      out.flush();
      for (int i = 1; i <= minFetches; i++) {
        ThreadWrapper<MigratableProcess> mpt = processes.get(processesSize - i);
        mpt.getTarget().suspend();
      }

      StringBuilder msg = new StringBuilder();
      for (int i = 1; i <= minFetches; i++) {
        ThreadWrapper<MigratableProcess> mpt = processes.get(processesSize - i);
        mpt.join();

        msg.append(SerializableWrite.objWrite(mpt.getTarget(), System.getProperty("user.dir"))
                + "\n");
      }
      out.writeBytes(msg.toString());
      out.flush();
      processes.refreshStatus();
    } catch (IOException e) {
      System.out.println("Connection refused, exit...");
      System.exit(0);
    } catch (InterruptedException e) {
      System.out.println("InterruptedException, exit...");
      System.exit(0);
    }
  }

  /**
   * Keep receiving the addresses of the objects that needs to be resumed
   * 
   * @param master
   *          the socket of the master node
   * @param numReqs
   *          number of processes that need to be resumed
   * @throws IOException
   * @throws ClassNotFoundException
   */
  private void processOfferObjCall(final BufferedReader in, final int numReqs) {
    try {
      for (int i = 0; i < numReqs; i++) {
        String fileName = in.readLine();
        ObjectInputStream inStream = new ObjectInputStream(new FileInputStream(fileName));

        // resume the process issued from the master
        MigratableProcess mp = (MigratableProcess) inStream.readObject();
        ThreadWrapper<MigratableProcess> mpt = new ThreadWrapper<MigratableProcess>(mp);
        mpt.start();

        System.out.println("Resuming " + mp.toString());
        processes.add(mpt);
        new File(fileName).delete();
      }
    } catch (IOException e) {
      System.out.println("Serializable Object cannot be found: " + e);
    } catch (ClassNotFoundException e) {
      System.out.println("Class is not found in the build path: " + e);
    }
  }
}
