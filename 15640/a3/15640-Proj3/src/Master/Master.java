package Master;

import CommandLineService.CmdLineProcessor;
import Configuration.ConfigParser;

/**
 * Class: Master.java
 * 
 * This serves as the entry point of the master process
 * 
 * @author Yang Sun
 * 
 */
public class Master {

  private ConfigParser configParser;

  private CmdLineProcessor cmdProcessor;

  private MrService mrService;

  public Master(String properties) {
    configParser = new ConfigParser(properties);

    /*
     * Prepare the Map-Reducer framework. Note that it is running at the background to receive
     * registration from client, but the core functions are not able to run without a start command.
     */
    mrService = new MrService(configParser);
    new Thread(mrService).start();

    /* Start the command line processor to interact with users' input */
    cmdProcessor = new CmdLineProcessor("Master", mrService, configParser);
    cmdProcessor.run();
  }

  public static void main(String[] args) {
    if (args.length != 1) {
      System.out.println("Usage: Master <Properties File>");
      System.exit(0);
    }
    new Master(args[0]);
    System.exit(0);
  }
}
