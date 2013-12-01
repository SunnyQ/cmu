package Examples;

import java.io.IOException;

import MapReduce.IO.OutputRecordWriter;
import MapReduce.Implementation.StringOutputRecord;

/**
 * Class: StringOutputWrite.java
 * 
 * An illustration for the user how to encrypt the input file for the mapper use
 * 
 * @author Yuan Gu
 * 
 */
public class StringOutputWrite {

  public static void main(String[] args) {
    OutputRecordWriter outputRecordWriter = null;
    try {
      outputRecordWriter = new OutputRecordWriter("test.input", 0, 16);
      StringOutputRecord record = new StringOutputRecord();
      int n = 1000;
      while (n-- > 0) {
        record.setValue("" + n);
        outputRecordWriter.appendRecord(record);
      }

    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (outputRecordWriter != null)
        try {
          outputRecordWriter.close();
        } catch (IOException e) {
        }
    }
  }
}
