package Examples;

import java.io.IOException;
import java.util.Iterator;

import MapReduce.IO.OutputRecordWriter;
import MapReduce.Implementation.StringOutputRecord;
import MapReduce.Implementation.StringWritable;
import MapReduce.Interface.Reducer;

/**
 * Class: StringInputReducer.java
 * 
 * A reducer example to output the sum of all numbers
 * 
 * @author Yuan Gu
 * 
 */
public class StringInputReducer implements Reducer<StringWritable, StringWritable> {

  @Override
  public void Reduce(StringWritable key, Iterator<StringWritable> values, OutputRecordWriter writer) {
    long total = 0;
    while (values.hasNext()) {
      String val = values.next().toString();
      System.out.println(val);
      total += Long.parseLong(val);
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
