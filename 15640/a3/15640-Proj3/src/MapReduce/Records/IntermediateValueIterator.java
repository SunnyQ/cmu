package MapReduce.Records;

import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Iterator;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateValueIterator.java
 * 
 * The iterator that iterates on each intermediate value
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateValueIterator<V extends Value> implements Iterator<V>, Closeable {

  private long recordNum;

  private long processedRecordNum;

  private IntermediateRecordIterator<? extends Key, V> intermediateRecordIterator = null;

  public IntermediateValueIterator(
          IntermediateRecordIterator<? extends Key, V> intermediateRecordIterator, long recordNum)
          throws FileNotFoundException, IOException {
    this.intermediateRecordIterator = intermediateRecordIterator;
    this.recordNum = recordNum;
    this.processedRecordNum = 0;
  }

  @Override
  public boolean hasNext() {
    if (this.intermediateRecordIterator.hasNext() && this.processedRecordNum < this.recordNum)
      return true;
    return false;
  }

  @Override
  public V next() {
    if (hasNext()) {
      this.processedRecordNum++;
      return this.intermediateRecordIterator.next().getValue();
    }

    return null;
  }

  @Override
  public void remove() {
    throw new UnsupportedOperationException();
  }

  @Override
  public void close() throws IOException {
    if (this.intermediateRecordIterator != null)
      this.intermediateRecordIterator.close();
  }

  public long getRecordNum() {
    return recordNum;
  }

  public void setRecordNum(long recordNums) {
    this.recordNum = recordNums;
  }

}
