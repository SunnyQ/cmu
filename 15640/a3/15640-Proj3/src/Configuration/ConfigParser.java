package Configuration;

import java.io.FileInputStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * Class: ConfigParser.java
 * 
 * This serves as a tool to parse the configuration properties file
 * 
 * @author Yang Sun
 * 
 */
public class ConfigParser {

  private Properties prop = new Properties();

  private Map<String, String> clientAddrMap = new HashMap<String, String>();

  private Map<String, Integer> clientPortMap = new HashMap<String, Integer>();

  public ConfigParser(String proportyFile) {
    try {
      prop.load(new FileInputStream(proportyFile));
      String[] clients = prop.getProperty("clients").split(",");
      for (int i = 0; i < clients.length; i++) {
        clientAddrMap.put(clients[i], InetAddress.getByName(prop.getProperty(clients[i]))
                .getHostAddress());
        clientPortMap.put(clients[i], Integer.parseInt(prop.getProperty(clients[i] + "_port")));
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public int getMaxMaps() {
    return Integer.parseInt(prop.getProperty("max_maps"));
  }

  public int getMaxReduces() {
    return Integer.parseInt(prop.getProperty("max_reduces"));
  }

  public String getMasterAddr() {
    try {
      return InetAddress.getByName(prop.getProperty("master")).getHostAddress();
    } catch (UnknownHostException e) {
      e.printStackTrace();
    }
    return null;
  }

  public int getMasterPort() {
    return Integer.parseInt(prop.getProperty("master_port"));
  }

  public Map<String, String> getClientAddrs() {
    return clientAddrMap;
  }

  public String getClientAddr(String clientID) {
    return getClientAddrs().get(clientID);
  }

  public Map<String, Integer> getClientPorts() {
    return clientPortMap;
  }

  public int getClientPort(String clientID) {
    return getClientPorts().get(clientID);
  }

}
