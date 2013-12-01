package MapReduce.Implementation;

import MapReduce.Interface.Key;
import MapReduce.Interface.Partitioner;
import MapReduce.Interface.Value;

/**
 * Class: HashPartitioner.java
 * 
 * The HashPartitioner used in the mapper task
 * 
 * @author Yuan Gu
 * 
 */
public class HashPartitioner<K extends Key, V extends Value> implements Partitioner<K, V> {
  @Override
  public int getPartitionID(K key, V value, int numPartitions) {
    return key.hashCode() % numPartitions;
  }
}
