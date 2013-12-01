/*
 * 
 */
package MapReduce.IO;

import java.io.Closeable;
import java.io.IOException;
import java.io.RandomAccessFile;

import MapReduce.Records.InputRecord;

/**
 * Class: InputRecordReader.java
 * 
 * This serves as a tool to read the input records
 * 
 * @author Yuan Gu
 * 
 */
public class InputRecordReader<T extends InputRecord> implements Closeable {

  private RandomAccessFile file;

  private int inputRecordLength;

  private Class<T> inputRecordClass;

  public InputRecordReader(String filePath, long offset, Class<T> inputRecordClass,
          int inputRecordLength) throws IOException {
    this.file = new RandomAccessFile(filePath, "r");
    this.file.seek(offset);
    this.inputRecordLength = inputRecordLength;
    this.inputRecordClass = inputRecordClass;
  }

  /**
   * Read bytes from the file
   * 
   * @param recordLen
   * @return byte array
   * @throws IOException
   */
  private byte[] readBytes(int recordLen) throws IOException {
    byte[] buf = new byte[recordLen];
    this.file.readFully(buf);
    return buf;
  }

  public void close() throws IOException {
    this.file.close();
  }

  /**
   * Return the next record
   * 
   * @return
   */
  public T getNextRecord() {
    byte[] buf = null;
    T record = null;
    try {
      buf = this.readBytes(this.inputRecordLength);
      record = this.inputRecordClass.newInstance();
      record.parseFromBytes(buf);
    } catch (Exception e) {
    }
    return record;
  }
}
