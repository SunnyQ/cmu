package CommandLineService;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

import Communication.Message;
import Communication.ServiceRunnable;
import Communication.StringMessagePayload;
import Configuration.ConfigParser;

/**
 * Class: CmdLineProcessor.java
 * 
 * This serves as the service to interact with user command line input
 * 
 * @author Yang Sun
 * 
 */
public class CmdLineProcessor {

  private String[] cmdLine;

  private BufferedReader in;

  private ServiceRunnable service;

  private ConfigParser configParser;

  private String hostID;

  public CmdLineProcessor(String hostID, ServiceRunnable service, ConfigParser configParser) {
    this.setHostID(hostID);
    this.service = service;
    in = new BufferedReader(new InputStreamReader(System.in));
    this.configParser = configParser;
  }

  public void run() {
    try {
      do {
        System.out.print("==> ");
        String curLine = in.readLine();
        if (curLine == null)
          continue;
        cmdLine = curLine.split(" ");

        /* Process the submit command, do error checking and send the command to the master */
        if (cmdLine[0].equalsIgnoreCase("submit")) {
          if (cmdLine.length != 6) {
            System.out
                    .println("Usage: submit <jarFile> <MapClass> <InputFile> <ReduceClass> <OutputFolder>");
            continue;
          }

          if (!new File(cmdLine[1]).exists()) {
            System.out.println(cmdLine[1] + " doesn't exist!");
            continue;
          }
          
          if (!new File(cmdLine[3]).exists()) {
            System.out.println(cmdLine[3] + " doesn't exist!");
            continue;
          }

          /* Prepare the message */
          StringMessagePayload payload = new StringMessagePayload(curLine.substring(curLine
                  .indexOf(" ") + 1));
          payload.setAttr(hostID);
          final Message msg = new Message(Message.TYPE.SUBMIT, payload);

          /* Send in a new thread so that it won't block the user input */
          new Thread(new Runnable() {
            @Override
            public void run() {
              try {
                Message responseMsg = Message.sendRequest(configParser.getMasterAddr(),
                        configParser.getMasterPort(), 0, msg);
                if (responseMsg.getType() == Message.TYPE.SUBMIT) {
                  System.out.println(((StringMessagePayload) responseMsg.getPayload()).getMsg());
                }
              } catch (Exception e) {
                e.printStackTrace();
              }
            }
          }).start();
        }

        /* Process the start command, send to master and print out the response */
        else if (cmdLine[0].equalsIgnoreCase("start")) {
          Message msg = new Message(Message.TYPE.START, new StringMessagePayload("START"));
          Message responseMsg = Message.sendRequest(configParser.getMasterAddr(),
                  configParser.getMasterPort(), 5000, msg);
          System.out.println(((StringMessagePayload) responseMsg.getPayload()).getMsg());
        }

        /* Process the stop command, send to master and print out the response */
        else if (cmdLine[0].equalsIgnoreCase("stop")) {
          Message msg = new Message(Message.TYPE.STOP, new StringMessagePayload("STOP"));
          Message responseMsg = Message.sendRequest(configParser.getMasterAddr(),
                  configParser.getMasterPort(), 5000, msg);
          System.out.println(((StringMessagePayload) responseMsg.getPayload()).getMsg());
        }

        /* Process the monitor command, send to master and print out the result */
        else if (cmdLine[0].equalsIgnoreCase("monitor")) {
          Message msg = new Message(Message.TYPE.MONITOR, new StringMessagePayload(hostID));
          Message responseMsg = Message.sendRequest(configParser.getMasterAddr(),
                  configParser.getMasterPort(), 5000, msg);
          System.out.println(((StringMessagePayload) responseMsg.getPayload()).getMsg());
        }

        /* Turn the service off if user enters "quit" */
      } while (!cmdLine[0].equalsIgnoreCase("quit"));

      /* Shut down the MrService and RequestHandlingService as well */
      service.shutdown();

    } catch (IOException e) {
      e.printStackTrace();
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    }
  }

  public String getHostID() {
    return hostID;
  }

  public void setHostID(String hostID) {
    this.hostID = hostID;
  }
}
