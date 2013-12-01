package Examples;

import java.io.IOException;
import java.util.Iterator;

import MapReduce.IO.OutputRecordWriter;
import MapReduce.Implementation.StringOutputRecord;
import MapReduce.Implementation.StringWritable;
import MapReduce.Interface.Reducer;

/**
 * Class: OddNumberGrepReducer.java
 * 
 * A reducer example to count the total number of odds
 * 
 * @author Yang Sun
 * 
 */
public class OddNumberGrepReducer implements Reducer<StringWritable, StringWritable> {

  @Override
  public void Reduce(StringWritable key, Iterator<StringWritable> values, OutputRecordWriter writer) {
    long total = 0;
    while (values.hasNext()) {
      values.next();
      total++;
    }

    System.out.println(key + " total " + total);
    StringOutputRecord record = new StringOutputRecord();
    record.setValue(key + " total " + total);
    try {
      writer.appendRecord(record);
    } catch (IOException e) {
      e.printStackTrace();
    }

  }

  @Override
  public int getRecordLength() {
    return 128;
  }

}
