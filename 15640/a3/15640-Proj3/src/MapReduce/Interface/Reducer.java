package MapReduce.Interface;

import java.util.Iterator;

import MapReduce.IO.OutputRecordWriter;

/**
 * Interface: Reducer.java
 * 
 * The Reducer interface where the user program has to extend from
 * 
 * @author Yuan Gu
 * 
 */
public interface Reducer<K extends Key, V extends Value> {
  public abstract void Reduce(K key, Iterator<V> values, OutputRecordWriter writer);

  public int getRecordLength();
}
