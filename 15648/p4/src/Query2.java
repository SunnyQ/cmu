import java.util.ArrayList;
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

public class Query2 {

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
    
    Scanner scan = connector.createScanner(finalTable, new Authorizations());
    scan.setRange(new Range(args[1] + delimiter + args[2], args[1] + delimiter + args[3]));
    scan.fetchColumnFamily(new Text("size"));

    Map<String, ArrayList<Long>> fileMap = new HashMap<String, ArrayList<Long>>();
    for (Entry<Key, Value> e : scan) {
      String row = e.getKey().getRow().toString();
      Long size = Long.parseLong(e.getValue().toString());
      String path = row.split(delimiter)[2];

      ArrayList<Long> sizeList = fileMap.get(path);
      if (sizeList == null) {
        sizeList = new ArrayList<Long>();
        sizeList.add(0L);
        sizeList.add(0L);
        fileMap.put(path, sizeList);
      }
      if (e.getKey().getColumnQualifier().toString().equals("write")) {
        sizeList.set(0, sizeList.get(0) + size);
      } else {
        sizeList.set(1, sizeList.get(1) + size);
      }
    }

    long total = 0L;
    for (ArrayList<Long> e : fileMap.values()) {
      Long wSize = e.get(0);
      Long rSize = e.get(1);

      if (wSize > 0 && rSize > 0) {
        total += wSize + rSize;
      }
    }

    System.out.println(args[1] + " access " + total
            + " bytes data written and read back in the period " + args[2] + " to " + args[3]);
  }

}