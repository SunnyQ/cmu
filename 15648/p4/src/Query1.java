import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.accumulo.core.client.AccumuloException;
import org.apache.accumulo.core.client.AccumuloSecurityException;
import org.apache.accumulo.core.client.Connector;
import org.apache.accumulo.core.client.MutationsRejectedException;
import org.apache.accumulo.core.client.Scanner;
import org.apache.accumulo.core.client.TableExistsException;
import org.apache.accumulo.core.client.TableNotFoundException;
import org.apache.accumulo.core.client.ZooKeeperInstance;
import org.apache.accumulo.core.data.Key;
import org.apache.accumulo.core.data.Range;
import org.apache.accumulo.core.data.Value;
import org.apache.accumulo.core.security.Authorizations;
import org.apache.hadoop.io.Text;

public class Query1 {

  private static final String finalTable = "yksun_finaltab";

  private static final String username = "yksun";

  private static final String password = "19881015";

  private static final String zookeeperInstance = "accumulo";

  private static final String zookeeperServer = "zk2:2181,zk3:2181,zk4:2181";

  private static final String delimiter = "~";

  private static Connector connector;

  public static void main(String[] args) throws AccumuloException, AccumuloSecurityException,
          TableExistsException, TableNotFoundException, MutationsRejectedException {
    connector = new ZooKeeperInstance(zookeeperInstance, zookeeperServer).getConnector(username,
            password.getBytes());
    
    ArrayList<String> ugiList = new ArrayList<String>();
    Scanner scan = connector.createScanner(finalTable, new Authorizations());
    for (Entry<Key, Value> e : scan) {
      String ugi = e.getKey().getRow().toString().split(delimiter)[0];
      if (!ugiList.contains(ugi)) {
        ugiList.add(ugi);
        printResult(ugi);
      }
    }
  }

  private static void printResult(String curUgi) throws AccumuloException,
          AccumuloSecurityException, TableExistsException, TableNotFoundException,
          MutationsRejectedException {
    Scanner scan = connector.createScanner(finalTable, new Authorizations());
    scan.setRange(new Range(new Text(curUgi), new Text(curUgi + "~9999")));
    scan.fetchColumnFamily(new Text("size"));

    Map<String, Long> fileMap = new HashMap<String, Long>();
    for (Entry<Key, Value> e : scan) {
      String row = e.getKey().getRow().toString();
      Long size = Long.parseLong(e.getValue().toString());
      String path = row.split(delimiter)[2];

      Long newSize = fileMap.get(path);
      if (newSize == null)
        newSize = 0L;
      newSize += size;
      fileMap.put(path, newSize);
    }

    curUgiOutput(curUgi, fileMap);
  }

  private static void curUgiOutput(String curUgi, Map<String, Long> fileMap) {
    ArrayList<Integer> interest = getInterestedPoints(fileMap.size() - 1);
    ArrayList<Long> valSet = new ArrayList<Long>(fileMap.values());
    Collections.sort(valSet);
    StringBuilder builder = new StringBuilder(curUgi);
    long preVal = 0L;
    long curVal = 0L;
    for (int p : interest) {
      curVal = Math.round(Math.log10(valSet.get(p)));
      builder.append("\t");
      builder.append(curVal - preVal);
      preVal = curVal;
    }
    System.out.println(builder.toString());
  }

  private static ArrayList<Integer> getInterestedPoints(int numFiles) {
    ArrayList<Integer> interested = new ArrayList<Integer>();
    interested.add((int) Math.round(0.0 * numFiles));
    interested.add((int) Math.round(0.00001 * numFiles));
    interested.add((int) Math.round(0.0001 * numFiles));
    interested.add((int) Math.round(0.001 * numFiles));
    interested.add((int) Math.round(0.01 * numFiles));
    interested.add((int) Math.round(0.1 * numFiles));
    interested.add((int) Math.round(0.25 * numFiles));
    interested.add((int) Math.round(0.5 * numFiles));
    interested.add((int) Math.round(0.75 * numFiles));
    interested.add((int) Math.round(0.9 * numFiles));
    interested.add((int) Math.round(0.99 * numFiles));
    interested.add((int) Math.round(0.999 * numFiles));
    interested.add((int) Math.round(0.9999 * numFiles));
    interested.add((int) Math.round(0.99999 * numFiles));
    interested.add((int) Math.round(1.0 * numFiles));
    return interested;
  }
}
