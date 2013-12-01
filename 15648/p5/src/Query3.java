import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
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

public class Query3 {

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
    scan.fetchColumnFamily(new Text("deletor"));
    for (Entry<Key, Value> e : scan) {
      String ugi = e.getKey().getRow().toString().split(delimiter)[0];
      if (!ugiList.contains(ugi)) {
        ugiList.add(ugi);
        printResult(ugi, args[1], args[2]);
      }
    }
  }

  private static void printResult(String curUgi, String start, String end)
          throws AccumuloException, AccumuloSecurityException, TableExistsException,
          TableNotFoundException, MutationsRejectedException {
    Scanner scan = connector.createScanner(finalTable, new Authorizations());
    scan.setRange(new Range(curUgi + delimiter + start, curUgi + delimiter + end));
    scan.fetchColumnFamily(new Text("size"));
    scan.fetchColumnFamily(new Text("deletor"));

    Map<String, ArrayList<Long>> fileMap = new HashMap<String, ArrayList<Long>>();
    for (Entry<Key, Value> e : scan) {
      String row = e.getKey().getRow().toString();
      String path = row.split(delimiter)[2];

      ArrayList<Long> info = fileMap.get(path);
      if (info == null) {
        info = new ArrayList<Long>();
        fileMap.put(path, info);
        info.add(0L);
        info.add(0L);
        info.add(0L);
      }

      if (e.getKey().getColumnFamily().toString().equals("deletor")) {
        info.set(2, 1L);
      } else if (e.getKey().getColumnFamily().toString().equals("size")) {
        if (e.getKey().getColumnQualifier().toString().equals("write")) {
          info.set(0, info.get(0) + Long.parseLong(e.getValue().toString()));
        } else {
          info.set(1, info.get(1) + Long.parseLong(e.getValue().toString()));
        }
      }
    }

    Collection<String> unsorted = fileMap.keySet();
    List<String> sorted = asSortedList(unsorted);
    String dPath = null;
    for (String filePath : sorted) {
      if (dPath != null && filePath.startsWith(dPath))
        fileMap.get(filePath).set(2, 1L);

      if (fileMap.get(filePath).get(2) == 1L)
        dPath = filePath;
    }

    long total = 0L;
    for (ArrayList<Long> info : fileMap.values()) {
      if (info.get(2) == 1L && info.get(0) > 0L) {
        total += info.get(0) + info.get(1);
      }
    }

    if (total > 0L)
      System.out.println(curUgi + " access " + total
              + " bytes where the file gets created and deleted in the period " + start + " to "
              + end);
  }

  public static <T extends Comparable<? super T>> List<T> asSortedList(Collection<T> c) {
    List<T> list = new ArrayList<T>(c);
    java.util.Collections.sort(list);
    return list;
  }

}
