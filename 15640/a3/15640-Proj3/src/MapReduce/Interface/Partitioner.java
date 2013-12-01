package MapReduce.Interface;

/**
 * Interface: Partitioner.java
 * 
 * The Partitioner interface where the user program can extend from
 * 
 * @author Yuan Gu
 * 
 */
public interface Partitioner<K extends Key, V extends Value> {
  public abstract int getPartitionID(K key, V value, int numPartitions);
}
