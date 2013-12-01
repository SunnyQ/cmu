package Master;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import Communication.Message;
import Communication.ServiceRunnable;
import Communication.StringMessagePayload;
import Communication.TaskPayload;
import Configuration.ConfigParser;

/**
 * Class: StatusCheckService.java
 * 
 * The tool frequently check the running status of the client every 5s. If any exception happens on
 * the client, the master controller will remove it and transfer tasks to other clients until the
 * dead client recovers back.
 * 
 * @author Yang Sun
 * 
 */
public class StatusCheckService implements ServiceRunnable {
  private volatile boolean terminating;

  private volatile ArrayList<Thread> runningThreads;

  private boolean isAlive;

  private Map<String, List<TaskPayload>> clients;

  private ConfigParser parser;

  public class StatusCmdIssuer implements Runnable {

    private String id;

    public StatusCmdIssuer(ConfigParser parser, String id) {
      this.id = id;
    }

    @Override
    public void run() {
      try {
        /* Ping the client and see if it is healthy. */
        Message msg = new Message(Message.TYPE.STATUS, new StringMessagePayload("QUERY"));
        Message response = Message.sendRequest(parser.getClientAddr(id), parser.getClientPort(id),
                5000, msg);
        if (response.getType() == Message.TYPE.STATUS) {
          StringMessagePayload payload = (StringMessagePayload) response.getPayload();
          if (payload.getMsg().equals("HEALTHY")) {
            System.out.println(id + " is healthy.");
          }
        }
      } catch (Exception e) {
        /* If there is no reply, do the exception handling. */
        MrService.exceptionHandler(id);
      }
    }
  }

  public StatusCheckService(Map<String, List<TaskPayload>> clients, ConfigParser parser) {
    runningThreads = new ArrayList<Thread>();
    this.clients = clients;
    this.parser = parser;
  }

  @Override
  public void run() {
    isAlive = true;

    while (!terminating) {
      try {
        for (String id : clients.keySet()) {
          Thread t = new Thread(new StatusCmdIssuer(parser, id));
          runningThreads.add(t);
          t.start();
        }

        for (Thread t : runningThreads)
          t.join();
        runningThreads.clear();

        /* issue the command every 5 sec. */
        Thread.sleep(5000);

      } catch (InterruptedException e) {
        e.printStackTrace();
      }
    }
    terminating = false;
  }

  @Override
  public void shutdown() throws IOException {
    if (!isAlive) {
      return;
    }

    System.out.println("Terminating signal received...");
    terminating = true;
    while (terminating)
      ;
    isAlive = false;
    System.out.println("StatusCheckService quiting...");
  }

  @Override
  public boolean isAlive() {
    return isAlive;
  }

}
