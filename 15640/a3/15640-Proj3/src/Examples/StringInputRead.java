package Examples;

import java.io.IOException;

import MapReduce.IO.InputRecordReader;
import MapReduce.Implementation.StringInputRecord;
import MapReduce.Records.InputRecord;

/**
 * Class: StringInputRead.java
 * 
 * An illustration for the user how to decrypt the reducer output file
 * 
 * @author Yuan Gu
 * 
 */
public class StringInputRead {

  public static void main(String[] args) {

    InputRecordReader<StringInputRecord> inputRecordReader = null;
    try {
      inputRecordReader = new InputRecordReader<StringInputRecord>(
              "reduce-output/output_1365892334967_client_2", 0,
              StringInputRecord.class, 128);
      InputRecord record = null;
      while ((record = inputRecordReader.getNextRecord()) != null) {
        System.out.println(record);
      }
      inputRecordReader.close();
    } catch (IOException e) {
    } finally {
      if (inputRecordReader != null)
        try {
          inputRecordReader.close();
        } catch (IOException e) {
        }
    }
  }
}
