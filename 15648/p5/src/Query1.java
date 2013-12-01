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

  private static Map<String, Long> all_fileMap = new HashMap<String, Long>();

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
        printResult(ugi, args[1], args[2]);
      }
    }

    if (all_fileMap.size() > 0)
      curUgiOutput("All", all_fileMap);
  }

  private static void printResult(String curUgi, String start, String end)
          throws AccumuloException, AccumuloSecurityException, TableExistsException,
          TableNotFoundException, MutationsRejectedException {
    Scanner scan = connector.createScanner(finalTable, new Authorizations());
    scan.setRange(new Range(new Text(curUgi + delimiter + start),
            new Text(curUgi + delimiter + end)));
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

      Long all_newSize = all_fileMap.get(path);
      if (all_newSize == null)
        all_newSize = 0L;
      all_newSize += size;
      all_fileMap.put(path, all_newSize);

    }

    if (fileMap.size() > 0)
      curUgiOutput(curUgi, fileMap);
  }

  private static void curUgiOutput(String curUgi, Map<String, Long> fileMap) {
    Long total = 0L;
    ArrayList<Integer> interest = getInterestedPoints(fileMap.size() - 1);
    ArrayList<Long> valSet = new ArrayList<Long>(fileMap.values());
    Collections.sort(valSet);
    StringBuilder builder = new StringBuilder(curUgi);
    double preVal = 0.0;
    double curVal = 0.0;

    for (long size : fileMap.values())
      total += size;

    for (int p : interest) {
      curVal = Math.log10(valSet.get(p));
      builder.append("\t");
      builder.append(curVal - preVal);
      preVal = curVal;
    }
    builder.append("\t");
    builder.append(Math.log10(total) - preVal);
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
