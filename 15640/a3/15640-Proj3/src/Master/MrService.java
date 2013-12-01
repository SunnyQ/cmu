package Master;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import util.JarExtractor;
import Communication.MapperTaskPayload;
import Communication.Message;
import Communication.ReducerTaskPayload;
import Communication.ServiceRunnable;
import Communication.StringMessagePayload;
import Communication.TaskPayload;
import Configuration.ConfigParser;
import MapReduce.Interface.Mapper;

/**
 * Class: MrService.java
 * 
 * This serves as the real map-reduce service running on the master controller Need start command to
 * turn it on.
 * 
 * @author Yang Sun
 * 
 */
public class MrService implements ServiceRunnable {

  private static ConfigParser parser;

  private int maxMaps;

  private int maxReduces;

  private int masterPort;

  private volatile ServerSocket serverSock;

  private boolean isAlive;

  private static volatile Map<String, List<TaskPayload>> clients;

  private static volatile Map<Long, List<Map<Integer, String>>> taskResultCollector;

  private StatusCheckService statusCheckService;

  public MrService(ConfigParser configParser) {
    parser = configParser;
    clients = new HashMap<String, List<TaskPayload>>();
    taskResultCollector = new HashMap<Long, List<Map<Integer, String>>>();
    maxMaps = configParser.getMaxMaps();
    maxReduces = configParser.getMaxReduces();
    masterPort = configParser.getMasterPort();
    isAlive = false;
    try {
      serverSock = new ServerSocket(masterPort);
    } catch (IOException e) {
      e.printStackTrace();
    }
    statusCheckService = new StatusCheckService(clients, configParser);
  }

  @Override
  public void run() {
    try {
      for (;;) {
        Socket clientSock = serverSock.accept();
        ObjectInputStream in = new ObjectInputStream(clientSock.getInputStream());
        Message incomingMsg = (Message) in.readObject();

        switch (incomingMsg.getType()) {
          case REGISTER:
            /* Process the registration, simply add to clients table and relay back */
            StringMessagePayload registerPayload = (StringMessagePayload) incomingMsg.getPayload();
            clients.put(registerPayload.getMsg(), new ArrayList<TaskPayload>());
            break;

          case SUBMIT:
            /* Process the job submission, if the mr facility is not running, do nothing */
            StringMessagePayload submitPayload = (StringMessagePayload) incomingMsg.getPayload();
            if (!isAlive) {
              submitPayload.setMsg("The Map-Reduce facility has not been started yet.");
            } else
              processSubmit(submitPayload, clientSock);
            break;

          case START:
            /*
             * Process the start command, start the status checking service. Mr framework is running
             * now.
             */
            StringMessagePayload startPayload = (StringMessagePayload) incomingMsg.getPayload();
            if (startPayload.getMsg().equals("START")) {
              isAlive = true;
              startPayload.setMsg("Start Successfully!");
              new Thread(statusCheckService).start();
            }
            break;

          case STOP:
            /*
             * Process the stop command, shut down the status checking service. Mr framework is down
             * now. Note the already running Mr tasks keeps running, the stop command simply means:
             * Don't accept any new job submission.
             */
            StringMessagePayload stopPayload = (StringMessagePayload) incomingMsg.getPayload();
            if (stopPayload.getMsg().equals("STOP")) {
              isAlive = false;
              stopPayload.setMsg("Stop Successfully!");
              statusCheckService.shutdown();
            }
            break;

          case MONITOR:
            /*
             * Process the monitor command, do nothing if Mr is not running. If running, return the
             * actual number of processes running on the query client back.
             */
            StringMessagePayload monitorPayload = (StringMessagePayload) incomingMsg.getPayload();
            if (!isAlive) {
              monitorPayload.setMsg("The Map-Reduce facility has not been started yet.");
            } else {
              if (clients.containsKey(monitorPayload.getMsg())) {
                monitorPayload.setMsg(clients.get(monitorPayload.getMsg()).size() + "");
              } else {
                /* Master always runs 0 processes. */
                monitorPayload.setMsg(0 + "");
              }
            }
            break;

          case MAPPER:
            /*
             * This is the map task finish signal, process and send out the reduce tasks. Finish it
             * no matter whether the Mr is still running currently.
             */
            MapperTaskPayload mapperPayload = (MapperTaskPayload) incomingMsg.getPayload();
            processMapper(mapperPayload);
            break;

          case REDUCER:
            /*
             * This is the reduce task finish signal, process and send back the result to issuer.
             * Finish it no matter whether the Mr is still running currently.
             */
            ReducerTaskPayload reducerPayload = (ReducerTaskPayload) incomingMsg.getPayload();
            processReducer(reducerPayload);
            break;

          case EXCEPTION:
            /*
             * This is the exception signal sent from the map task or reduce task. That means, the
             * map or reduce user defined encounters a problem, the job cannot finished. Send
             * exception message back to the user.
             */
            TaskPayload exceptionPayload = (TaskPayload) incomingMsg.getPayload();
            processException(exceptionPayload);

          default:
            break;
        }

        /* Relay back the message */
        ObjectOutputStream out = new ObjectOutputStream(clientSock.getOutputStream());
        out.writeObject(incomingMsg);
        out.flush();
        out.close();
        clientSock.close();
      }
    } catch (IOException e) {
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
  }

  /**
   * Process the exception message from the map/reduce task.
   * 
   * @param exceptionPayload
   * @throws SocketTimeoutException
   * @throws UnknownHostException
   * @throws IOException
   * @throws ClassNotFoundException
   */
  private void processException(TaskPayload exceptionPayload) throws SocketTimeoutException,
          UnknownHostException, IOException, ClassNotFoundException {

    /* remove the tasks from the job tracking table */
    for (String c : clients.keySet()) {
      int i = 0;
      while (i < clients.get(c).size()) {
        if (clients.get(c).get(i).getTaskID() == exceptionPayload.getTaskID()) {
          clients.get(c).remove(i);
        } else {
          i++;
        }
      }
    }

    /* print out or send back the exception message to the task issuer */
    if (exceptionPayload.getIssuer().equals("Master")) {
      System.out.println("Task " + exceptionPayload.getTaskID() + " can't be finished.");
    } else {
      Message msg = new Message(Message.TYPE.RESULT, new StringMessagePayload("Task "
              + exceptionPayload.getTaskID() + " can't be finished."));
      String issuer = exceptionPayload.getIssuer();
      Message.sendRequest(parser.getClientAddr(issuer), parser.getClientPort(issuer), 5000, msg);
    }
  }

  /**
   * Process the reduce finish signal. Simply print out or send a message to the job issuer for the
   * completion.
   * 
   * @param reducerPayload
   */
  private void processReducer(ReducerTaskPayload reducerPayload) {

    /* Job is done on client #clientID, remove it from job tracking table */
    String clientID = reducerPayload.getClientID();
    boolean lastOne = removeTaskFromClients(clientID, reducerPayload.getTaskID());

    /* If all the reducers are done for the current taskID, finish up this task. */
    if (lastOne) {
      try {
        String issuer = reducerPayload.getIssuer();
        if (issuer.equals("Master"))
          System.out.println("Task " + reducerPayload.getTaskID() + " is finished.");
        else {
          Message msg = new Message(Message.TYPE.RESULT, new StringMessagePayload("Task "
                  + reducerPayload.getTaskID() + " is finished."));
          Message.sendRequest(parser.getClientAddr(issuer), parser.getClientPort(issuer), 5000, msg);
        }
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }

  /**
   * Process the map task finish signal. Simply print out or send a message to the job issuer for
   * the completion.
   * 
   * @param mapperPayload
   * @throws InterruptedException
   */
  private void processMapper(MapperTaskPayload mapperPayload) throws InterruptedException {

    /* remove the task from client task list in the job tracking table */
    boolean lastOne = removeTaskFromClients(mapperPayload.getClientID(), mapperPayload.getTaskID());

    /*
     * this means the current task has been failed on another client, this intermediate result can
     * be discard
     */
    if (!taskResultCollector.containsKey(mapperPayload.getTaskID()))
      return;

    /* the task runs so far so good, continue collect the intermediate result */
    List<Map<Integer, String>> taskResults = taskResultCollector.get(mapperPayload.getTaskID());
    taskResults.add(mapperPayload.getResultPartitions());

    /* all the results have been collected */
    if (lastOne) {
      List<Thread> runningThreads = new ArrayList<Thread>();
      List<String> freebies = getNFreeClients(mapperPayload.getResultPartitions().size());
      long taskID = System.currentTimeMillis();
      for (int i = 0; i < freebies.size(); i++) {

        /* Prepare the payload to send all reducer clients */
        ReducerTaskPayload reqPayload = new ReducerTaskPayload();
        reqPayload.setTaskID(taskID);
        reqPayload.setJarFile(mapperPayload.getJarFile());
        reqPayload.setReducerClass(mapperPayload.getReducerClass());
        reqPayload.setOutputFolder(mapperPayload.getOutputFolder());
        reqPayload.setReducerID(i);
        reqPayload.setIssuer(mapperPayload.getIssuer());
        reqPayload.setClientID(freebies.get(i));
        reqPayload.setInputSplits(new ArrayList<String>());

        for (int j = 0; j < taskResults.size(); j++)
          reqPayload.getInputSplits().add(taskResults.get(j).get(i));

        /* Send out the reduce tasks */
        Thread t = new Thread(new TaskRunnable(freebies.get(i), parser, new Message(
                Message.TYPE.REDUCER, reqPayload)));
        runningThreads.add(t);

        /*
         * Update the job tracking table, doesn't matter for the early addition. Exception handler
         * will update the job tracking table again if there is anything undesirable happens.
         */
        clients.get(freebies.get(i)).add(reqPayload);
        t.start();
      }

      /* Send all responsibilities to the reducer task, so free up the taskResultCollectors */
      taskResultCollector.remove(mapperPayload.getTaskID());

      try {
        for (Thread t : runningThreads)
          t.join();
      } catch (InterruptedException e) {
        e.printStackTrace();
      }
      runningThreads.clear();
    }
  }

  /**
   * Process job submission. Extract the classes from Jar File and send map tasks to clients
   * 
   * @param payload
   * @param clientSock
   */
  private void processSubmit(StringMessagePayload payload, Socket clientSock) {
    String[] args = payload.getMsg().split(" ");
    String jarFile = args[0];
    String mapperClass = args[1];
    String inputFile = args[2];
    String reducerClass = args[3];
    String outputFolder = args[4];

    try {

      /* Extract necessary information from the Jar file */
      JarExtractor extractor = new JarExtractor(jarFile);
      Class<?> mapCl = extractor.extractClass(mapperClass);
      Class<?> reduceCl = extractor.extractClass(reducerClass);

      if (mapCl == null || reduceCl == null) {
        payload.setMsg("Map Class or Reduce Class cannot be found in Jar.");
        return;
      }

      Mapper<?, ?, ?> mapper = (Mapper<?, ?, ?>) mapCl.newInstance();
      if (mapper.getNumMapper() > Math.min(maxMaps, clients.size())
              || mapper.getNumReducer() > Math.min(maxReduces, clients.size())) {
        payload.setMsg("The number of Maps or Reduces exceeds the number of slaves or maximum value allowance.");
        return;
      }

      /* Prepare sending the map tasks */
      List<Thread> runningThreads = new ArrayList<Thread>();
      List<String> freebies = getNFreeClients(mapper.getNumMapper());

      /* We use the current system time as the task ID to guarantee its unique. */
      long taskID = System.currentTimeMillis();

      taskResultCollector.put(taskID, new ArrayList<Map<Integer, String>>());
      for (int i = 0; i < freebies.size(); i++) {

        /* Prepare the payload and sent out the map tasks */
        MapperTaskPayload reqPayload = new MapperTaskPayload();
        reqPayload.setTaskID(taskID);
        reqPayload.setJarFile(jarFile);
        reqPayload.setMapperClass(mapperClass);
        reqPayload.setInputFile(inputFile);
        reqPayload.setOutputFolder(outputFolder);
        reqPayload.setReducerClass(reducerClass);
        reqPayload.setMapperID(i);
        reqPayload.setIssuer(payload.getAttr());
        reqPayload.setClientID(freebies.get(i));

        Thread t = new Thread(new TaskRunnable(freebies.get(i), parser, new Message(
                Message.TYPE.MAPPER, reqPayload)));
        runningThreads.add(t);

        /*
         * Update the job tracking table, doesn't matter for the early addition. Exception handler
         * will update the job tracking table again if there is anything undesirable happens.
         */
        clients.get(freebies.get(i)).add(reqPayload);
        t.start();
      }

      for (Thread t : runningThreads)
        t.join();
      runningThreads.clear();
      payload.setMsg("Task " + taskID + " is running...");

    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  /**
   * Handle any exceptions, restart the failed job on another client if possible.
   * 
   * @param clientID
   */
  public static void exceptionHandler(String clientID) {

    /* Jobs on this failed client have been processed already */
    if (!clients.containsKey(clientID))
      return;

    /* Collect all jobs running on this failed client */
    List<TaskPayload> remainingTasks = clients.get(clientID);
    clients.remove(clientID);
    String replacement = null;

    try {
      /* For each task, we optimistically look for a replacement client to run it */
      for (TaskPayload p : remainingTasks) {

        /* Look for a replacement client with least burden for the current task p */
        List<String> allClients = new ArrayList<String>(clients.keySet());
        Collections.sort(allClients, new Comparator<String>() {
          @Override
          public int compare(String o1, String o2) {
            return ((Integer) clients.get(o1).size()).compareTo((Integer) clients.get(o2).size());
          }
        });
        for (String c : allClients) {
          if (!containsTask(p.getTaskID(), clients.get(c))) {
            replacement = c;
            break;
          }
        }

        /* If there is nothing available, we can do nothing then... But let the user know! */
        if (replacement == null) {
          System.out.println("No substitution....");
          taskResultCollector.remove(p.getTaskID());
          if (p.getIssuer().equals("Master")) {
            System.out.println("Task " + p.getTaskID() + " is failed on " + p.getClientID());
          } else {
            Message.sendRequest(parser.getClientAddr(p.getIssuer()), parser.getClientPort(p
                    .getIssuer()), 5000, new Message(Message.TYPE.RESULT, new StringMessagePayload(
                    "Task " + p.getTaskID() + " is failed on " + p.getClientID())));
          }
          return;
        }

        /* OK, now we find the replacement client to accept the current job */
        clients.get(replacement).add(p);
        Message msg = null;
        if (p instanceof MapperTaskPayload) {
          ((MapperTaskPayload) p).setClientID(replacement);
          msg = new Message(Message.TYPE.MAPPER, p);
        } else if (p instanceof ReducerTaskPayload) {
          ((ReducerTaskPayload) p).setClientID(replacement);
          msg = new Message(Message.TYPE.REDUCER, p);
        } else {
          /* It is impossible to get here... */
        }
        Message.sendRequest(parser.getClientAddr(replacement), parser.getClientPort(replacement),
                5000, msg);
      }
    } catch (Exception e) {
      /* If the replacement client is failed again... OK, find another one... */
      exceptionHandler(replacement);
    }
  }

  /**
   * Remove a task from the client list in job tracking table once it is done
   * 
   * @param clientID
   * @param taskID
   * @return true if the task is all done
   */
  private boolean removeTaskFromClients(String clientID, long taskID) {
    boolean lastOne = true;
    for (String c : clients.keySet()) {
      if (c.equals(clientID)) {
        int i = 0;
        while (i < clients.get(c).size()) {
          if (clients.get(c).get(i).getTaskID() == taskID)
            clients.get(c).remove(i);
          else
            i++;
        }
      } else {
        /* See if there is same task running on other clients */
        for (int i = 0; i < clients.get(c).size(); i++)
          lastOne = lastOne && !(clients.get(c).get(i).getTaskID() == taskID);
      }
    }
    return lastOne;
  }

  /**
   * We want balance the tasks, so always get the clients with least burdens.
   * 
   * @param n
   * @return a list of clients available
   */
  public List<String> getNFreeClients(int n) {
    List<String> allClients = new ArrayList<String>(clients.keySet());

    /* sort depending on how many tasks running on each client */
    Collections.sort(allClients, new Comparator<String>() {
      @Override
      public int compare(String o1, String o2) {
        return ((Integer) clients.get(o1).size()).compareTo((Integer) clients.get(o2).size());
      }
    });

    return allClients.subList(0, n);
  }

  /**
   * Determine whether the list pool contains the task specified by taskID.
   * 
   * @param taskID
   * @param pool
   * @return true if exists, false otherwise
   */
  public static boolean containsTask(long taskID, List<TaskPayload> pool) {
    for (TaskPayload t : pool) {
      if (t.getTaskID() == taskID)
        return true;
    }
    return false;
  }

  public void shutdown() throws IOException {
    if (serverSock.isClosed())
      serverSock.close();
    isAlive = false;
  }

  public boolean isAlive() {
    return isAlive;
  }
}
