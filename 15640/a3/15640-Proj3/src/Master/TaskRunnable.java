package Master;

import Communication.Message;
import Configuration.ConfigParser;

/**
 * Class: TaskRunnable.java
 * 
 * The task threads for sending out the map tasks or reduce tasks from the master controller to
 * clients.
 * 
 * @author Yang Sun
 * 
 */
class TaskRunnable implements Runnable {
  private String clientID;

  private Message msg;

  private ConfigParser parser;

  public TaskRunnable(String clientID, ConfigParser parser, Message msg) {
    this.clientID = clientID;
    this.msg = msg;
    this.parser = parser;
  }

  @Override
  public void run() {
    try {
      /* Send task to the target client. */
      Message.sendRequest(parser.getClientAddr(clientID), parser.getClientPort(clientID), 0, msg);
    } catch (Exception e) {
      /* Do the exception handling for any errors occured. */
      MrService.exceptionHandler(clientID);
    }
  }
}