package Client;

import java.io.IOException;
import java.net.UnknownHostException;

import CommandLineService.CmdLineProcessor;
import Communication.Message;
import Communication.StringMessagePayload;
import Configuration.ConfigParser;

/**
 * Class: Client.java
 * 
 * This serves as the entry class of the client process
 * 
 * @author Yang Sun
 * 
 */
public class Client {

  private ConfigParser configParser;

  private CmdLineProcessor cmdProcessor;

  private RequestHandlingService rhs;

  public Client(String clientID, String properties) throws UnknownHostException, IOException {
    configParser = new ConfigParser(properties);

    /* Start RestartHandlingService to process master's request */
    rhs = new RequestHandlingService(clientID, configParser);
    new Thread(rhs).start();

    /* Register with Master Controller */
    registerWithMaster(clientID);

    /* Start Command Line Processor to interact with the user */
    cmdProcessor = new CmdLineProcessor(clientID, rhs, configParser);
    cmdProcessor.run();
  }

  /**
   * Send a message to the master to join the MapReduce framework
   * 
   * @param clientID
   *          the current client ID
   */
  private void registerWithMaster(String clientID) {
    Message msg = new Message(Message.TYPE.REGISTER, new StringMessagePayload(clientID));
    try {
      Message.sendRequest(configParser.getMasterAddr(), configParser.getMasterPort(), 5000, msg);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public static void main(String[] args) throws UnknownHostException, IOException {
    if (args.length != 2) {
      System.out.println("Usage: Client <Client ID> <Properties File>");
      System.exit(0);
    }
    new Client(args[0], args[1]);
    System.exit(0);
  }

}
