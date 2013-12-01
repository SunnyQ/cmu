package cmu.edu.ds.a1.Thread;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.ArrayList;
import java.util.List;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.SerializableWrite;
import cmu.edu.ds.a1.PM.Processes;
import cmu.edu.ds.a1.PM.Slaves;
import cmu.edu.ds.a1.PM.ThreadWrapper;
import cmu.edu.ds.a1.Thread.M2SThread.MasterAction;

/**
 * This serves as the main communication thread for master node and slave node
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class MasterCommandIssuerThread implements Runnable {

  private Slaves slaves;

  private List<String> masterStock;

  private List<Thread> runningThreads;

  private Processes processes;

  public MasterCommandIssuerThread(Slaves slaves, Processes processes) throws IOException {
    this.slaves = slaves;
    this.processes = processes;
    this.masterStock = new ArrayList<String>();
    this.runningThreads = new ArrayList<Thread>();
  }

  @Override
  public void run() {
    try {
      for (;;) {

        synchronized (processes) {
          synchronized (slaves) {
            int processesSize = processes.size();

            // issue fetchstat command to all slaves
            slaves.refreshStatus();
            issueFetchStats();

            // master node load balancing - otiose
            int curLoadLevel = slaves.getLoadLevel(processesSize);
            System.out.println("Current load level is " + curLoadLevel);
            if (processesSize > curLoadLevel)
              masterNodeOtioseBalance(curLoadLevel);

            // slave nodes load balancing - otiose
            slaves.refreshStatus();
            slaveNodeOtioseBalance(curLoadLevel);

            // master node load balancing - shortage
            if (processesSize < curLoadLevel && masterStock.size() > 0)
              masterNodeShortageBalance(curLoadLevel);

            // slave node load balancing - shortage
            slaves.refreshStatus();
            if (masterStock.size() > 0)
              slaveNodeShortageBalance(curLoadLevel);
          }
        }
        // use thread join to wait all complete.
        Thread.sleep(5000);
      }
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  /**
   * issue FETCHSTATS to all the slaves
   * 
   * @throws InterruptedException
   */
  private void issueFetchStats() throws InterruptedException {
    for (M2SThread slave : slaves) {
      Thread fetchStatThread = new Thread(slave.switchAction(MasterAction.FETCHSTATS));
      runningThreads.add(fetchStatThread);
      fetchStatThread.start();
    }
    waitForRunningThread();
  }

  /**
   * Deal with master end otiose processes
   * 
   * @param curLoadLevel
   *          the current balance load level
   * @throws InterruptedException
   */
  private void masterNodeOtioseBalance(int curLoadLevel) throws InterruptedException {
    int processesSize = processes.size();
    int otioseNum = processesSize - curLoadLevel;
    for (int i = 1; i <= otioseNum; i++) {
      MigratableProcess mp = processes.get(processesSize - i).getTarget();
      mp.suspend();
    }

    for (int i = 1; i <= otioseNum; i++) {
      ThreadWrapper<MigratableProcess> mpt = processes.get(processesSize - i);
      mpt.join();

      // write out the serialized object
      String tmpFileName = SerializableWrite.objWrite(mpt.getTarget(),
              System.getProperty("user.dir"));

      // this will implicitly remove dead process
      if (tmpFileName != null)
        masterStock.add(tmpFileName);
    }
    processes.refreshStatus();
  }

  /**
   * Deal with slave end otiose processes
   * 
   * @param curLoadLevel
   *          current balance load level
   * @throws InterruptedException
   */
  private void slaveNodeOtioseBalance(int curLoadLevel) throws InterruptedException {
    for (M2SThread slave : slaves) {
      if (slave.isAlive() && slave.getNumProcesses() > curLoadLevel) {
        slave.setFetchNum(slave.getNumProcesses() - curLoadLevel);
        Thread fetchObjThead = new Thread(slave.switchAction(MasterAction.FETCHOBJ));
        runningThreads.add(fetchObjThead);
        fetchObjThead.start();
      }
    }
    waitForRunningThread();

    // collect all otiose processes from slaves
    for (M2SThread slave : slaves) {
      if (slave.isAlive()) {
        masterStock.addAll(slave.getStocks());
        slave.getStocks().clear();
      }
    }
  }

  /**
   * Deal with master level shortage processes
   * 
   * @param curLoadLevel
   *          the current load balance level
   * @throws FileNotFoundException
   * @throws IOException
   * @throws ClassNotFoundException
   */
  private void masterNodeShortageBalance(int curLoadLevel) {
    try {
      int processesSize = processes.size();
      for (int i = 0; i < curLoadLevel - processesSize; i++) {
        if (masterStock.size() > 0) {
          // pick obiose processes from masterStock and launch it
          System.out.println(masterStock.get(0));
          ObjectInputStream in = new ObjectInputStream(new FileInputStream(masterStock.get(0)));
          MigratableProcess mp = (MigratableProcess) in.readObject();
          in.close();
          ThreadWrapper<MigratableProcess> mpt = new ThreadWrapper<MigratableProcess>(mp);
          mpt.start();

          System.out.println("Resuming " + mp.toString());
          processes.add(mpt);
          new File(masterStock.remove(0)).delete();
        }
      }
    } catch (FileNotFoundException e) {
      System.out.println("Serializable Object cannot be found: " + e);
    } catch (IOException e) {
      System.out.println("Serializable Object cannot be found: " + e);
    } catch (ClassNotFoundException e) {
      System.out.println("Class is not found in the build path: " + e);
    }
  }

  /**
   * Deal with slave end shortage processes
   * 
   * @param curLoadLevel
   *          the current balance load level
   * @throws InterruptedException
   */
  private void slaveNodeShortageBalance(int curLoadLevel) throws InterruptedException {
    for (M2SThread slave : slaves) {
      if (slave.isAlive() && slave.getNumProcesses() < curLoadLevel) {

        // the minimum number of iterations cannot exceed the items left
        // in masterStock
        int minIter = Math.min(curLoadLevel - slave.getNumProcesses(), masterStock.size());

        // relocate the process to the slave stock
        for (int i = 0; i < minIter; i++)
          slave.getStocks().add(masterStock.get(i));

        // start slave thread to finish transaction
        Thread offerObjThread = new Thread(slave.switchAction(MasterAction.OFFEROBJ));
        runningThreads.add(offerObjThread);
        offerObjThread.start();
      }
    }
    waitForRunningThread();

    // supporse masterStock should be empty
    assert masterStock.size() == 0;
    masterStock.clear();
  }

  /**
   * Wait for all threads to be finished
   * 
   * @param runningThreads
   * @throws InterruptedException
   */
  private void waitForRunningThread() throws InterruptedException {
    for (Thread t : runningThreads)
      t.join();
    runningThreads.clear();
  }

}
