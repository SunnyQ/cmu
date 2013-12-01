package MapReduce.Records;

import java.io.Closeable;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateRecordIterator.java
 * 
 * The iterator that iterates on each intermediate record in the collector
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecordIterator<K extends Key, V extends Value> implements
        Iterator<IntermediateRecord<K, V>>, Closeable {

  private int bufferSize = 1000;

  private ObjectInputStream in;

  public int getBufferSize() {
    return bufferSize;
  }

  public void setBufferSize(int bufferSize) {
    this.bufferSize = bufferSize;
  }

  public IntermediateRecordIterator(String filePath, int bufferSize) throws FileNotFoundException,
          IOException {
    this.in = new ObjectInputStream(new FileInputStream(filePath));
    this.bufferSize = bufferSize;
  }

  public IntermediateRecordIterator(String filePath) throws FileNotFoundException, IOException {
    this.in = new ObjectInputStream(new FileInputStream(filePath));
  }

  private List<IntermediateRecord<K, V>> records = new LinkedList<IntermediateRecord<K, V>>();

  private ListIterator<IntermediateRecord<K, V>> recordItr = null;

  @Override
  public boolean hasNext() {

    if (this.recordItr == null || !this.recordItr.hasNext()) {
      this.records.clear();
      int k = util.RecordOperation.loadBulkObjects(this.records, this.in, this.bufferSize);
      this.recordItr = this.records.listIterator();
      if (k == 0)
        return false;
    }

    return this.recordItr.hasNext();
  }

  @Override
  public IntermediateRecord<K, V> next() {
    if (this.recordItr == null || !this.recordItr.hasNext())
      return null;
    return this.recordItr.next();
  }

  @Override
  public void remove() {
    throw new UnsupportedOperationException();
  }

  @Override
  public void close() throws IOException {
    if (this.in != null)
      this.in.close();
  }

//  public static void main(String[] args) {
//
//    int sum = 0;
//    IntermediateRecordIterator<StringWritable, StringWritable> reader = null;
//    try {
//      reader = new IntermediateRecordIterator<StringWritable, StringWritable>(
//      // "/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/partition_0");
//              "/Users/htcbug/Documents/Homework/15640/hw3/reduce-tmp/records.sorted");
//      // "/Users/htcbug/Documents/Homework/15640/hw3/reduce-tmp/partition_test");
//      // "/Users/htcbug/Documents/Homework/15640/hw3/reduce-tmp/records.sorted");
//      while (reader.hasNext()) {
//        IntermediateRecord<StringWritable, StringWritable> record = reader.next();
//        System.out.println(record);
//        sum++;
//      }
//    } catch (Exception e) {
//      e.printStackTrace();
//    } finally {
//      if (reader != null)
//        try {
//          reader.close();
//        } catch (IOException e) {
//          e.printStackTrace();
//        }
//    }
//
//    System.out.println(sum);
//  }
}
