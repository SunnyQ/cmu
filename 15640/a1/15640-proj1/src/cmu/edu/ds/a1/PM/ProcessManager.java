package cmu.edu.ds.a1.PM;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.InetAddress;
import java.util.Arrays;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.Thread.MasterStarterThread;
import cmu.edu.ds.a1.Thread.SlaveThread;

/**
 * The ProcessManager accepts String arguments and launches the requested new processes on either
 * itself or peer nodes for load balancing.
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class ProcessManager {

  private static int serverPort;

  private static String hostAddr;

  private final String mpPrefix = "cmu.edu.ds.a1.MP.";

  /**
   * processes array is volatile so that processes transferred from other nodes can be launched
   */
  private volatile Processes processes;

  public enum PMRole {
    MASTER, SLAVE
  }

  public ProcessManager(final PMRole role) throws IOException, InterruptedException {
    this.processes = new Processes();
    final Runnable targetMode = (role == PMRole.MASTER) ? new MasterStarterThread(serverPort,
            processes) : new SlaveThread(hostAddr, serverPort, processes);

    // Start the mode specific thread
    new Thread(targetMode).start();

    System.out.println("Entering " + ((role == PMRole.MASTER) ? "master" : "slave") + " mode...");

    String curLine = ""; // Line read from standard in
    BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
    while (!(curLine.equals("quit"))) {
      System.out.print("==> ");
      curLine = in.readLine();

      if (curLine == null)
        continue;

      // Process "ps" command, print out local processes
      else if (curLine.equals("ps")) {
        if (processes.size() == 0)
          System.out.println("no running processes");
        for (int i = 0; i < processes.size(); i++)
          System.out.println(processes.get(i).getTarget().toString());
      }

      // Process new process command, launch the program
      else if (!(curLine.equals("quit"))) {
        String[] inputs = curLine.split(" ");
        MigratableProcess mp = invokeProcess(inputs);
        if (mp != null) {
          ThreadWrapper<MigratableProcess> mpt = new ThreadWrapper<MigratableProcess>(mp);
          synchronized (processes) {
            processes.add(mpt);
          }
          mpt.start();
        }
      }
    }
  }

  /**
   * Invoke the processes depending on the user input
   * 
   * @param inputs
   *          the command the user inputs
   * @return MigratableProcess if created successfully
   * 
   */
  private MigratableProcess invokeProcess(final String[] inputs) {
    String processName = inputs[0];
    Object[] processArgs = { Arrays.copyOfRange(inputs, 1, inputs.length) };
    try {
      // Creates the process using Java reflection API
      Class<?> cl = Class.forName(mpPrefix + processName);
      Constructor<?> constructor = cl.getConstructor(String[].class);
      MigratableProcess mp = (MigratableProcess) constructor.newInstance(processArgs);
      return mp;
    } catch (ClassNotFoundException e) {
      System.out.println("Class " + processName + " is not found in the build path " + mpPrefix);
    } catch (InstantiationException e) {
      System.out.println("Instantiation failed with arguments " + processArgs.toString());
    } catch (IllegalAccessException e) {
      System.out.println("IllegalAccessException occured");
    } catch (IllegalArgumentException e) {
      System.out.println("IllegalArgumentException occured: String[] arguments is expected");
    } catch (InvocationTargetException e) {
      System.out.println("InvocationTargetException occured");
    } catch (SecurityException e) {
      System.out.println("SecurityException occured");
    } catch (NoSuchMethodException e) {
      System.out.println("NoSuchMethodException occured");
    }
    return null;
  }

  /**
   * Command-line parser used to process argument inputs
   * 
   * @param args
   *          command-line arguments
   * @return true if process successfully
   */
  private static boolean cmdParser(final String[] args) {
    if (args.length != 2 && args.length != 4)
      return false;

    try {
      for (int i = 0; i < args.length; i++) {
        if (args[i].equals("-p"))
          serverPort = Integer.parseInt(args[++i]);
        else if (args[i].equals("-c"))
          hostAddr = InetAddress.getByName(args[++i]).getHostAddress();
      }
    } catch (Exception e) {
      return false;
    }

    return true;
  }

  /**
   * Entry point of ProcessManager
   * 
   * @param args
   *          command-line arguments
   * @throws IOException
   * @throws InterruptedException
   */
  public static void main(String[] args) throws IOException, InterruptedException {
    if (!cmdParser(args)) {
      System.out.println("Usage: ProcessManager [-c <hostname>] -p <serverPort>");
      System.exit(0);
    }

    new ProcessManager(args.length == 4 ? PMRole.SLAVE : PMRole.MASTER);
    System.exit(1);
  }
}
