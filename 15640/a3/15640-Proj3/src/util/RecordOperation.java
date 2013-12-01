package util;

import java.io.ObjectInputStream;
import java.util.List;

import MapReduce.Records.IntermediateRecord;

/**
 * Class: RecordOperation.java
 * 
 * A utility tool to manipulate record operation more convenient.
 * 
 * @author Yuan Gu
 * 
 */
public class RecordOperation {
  @SuppressWarnings({ "rawtypes", "unchecked" })
  public static int loadBulkObjects(List list, ObjectInputStream in, int K) {

    int num = 0;
    for (int i = 0; i < K; i++) {
      IntermediateRecord record = null;
      try {
        record = (IntermediateRecord) in.readObject();
      } catch (Exception e) {
      }
      if (record == null)
        break;

      list.add(record);
      num++;
    }
    return num;
  }
}
