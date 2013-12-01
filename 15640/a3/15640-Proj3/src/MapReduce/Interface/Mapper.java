package MapReduce.Interface;

import MapReduce.Records.InputRecord;
import MapReduce.Records.IntermediateRecordCollector;

/**
 * Interface: Mapper.java
 * 
 * The Mapper interface where the user program has to extend from
 * 
 * @author Yuan Gu
 * 
 */
public interface Mapper<T extends InputRecord, K extends Key, V extends Value> {
  public abstract void Map(T record, IntermediateRecordCollector<K, V> collector);

  public int getRecordLength();

  public int getNumMapper();

  public int getNumReducer();
}
