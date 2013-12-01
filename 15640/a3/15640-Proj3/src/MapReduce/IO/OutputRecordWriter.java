/*
 * 
 */
package MapReduce.IO;

import java.io.Closeable;
import java.io.IOException;
import java.io.RandomAccessFile;

import MapReduce.Records.OutputRecord;

/**
 * Class: OutputRecordWriter.java
 * 
 * This serves as a tool to write the output records to a file
 * 
 * @author Yuan Gu
 * 
 */
public class OutputRecordWriter implements Closeable {

  private RandomAccessFile file;

  private int recordLength;

  public OutputRecordWriter(String filePath, long offset, int recordLength) throws IOException {
    util.FileOperation.deleteFile(filePath);
    this.file = new RandomAccessFile(filePath, "rw");
    this.file.seek(offset);
    this.recordLength = recordLength;
  }

  public void close() throws IOException {
    this.file.close();
  }

  public void appendRecord(OutputRecord record) throws IOException {
    this.file.write(record.toBytes(this.recordLength));
  }
}
