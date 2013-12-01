package MapReduce.Records;

import java.io.Serializable;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateRecord.java
 * 
 * The intermediate record output from the map task
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecord<K extends Key, V extends Value> implements
        Comparable<IntermediateRecord<K, V>>, Serializable {
  private static final long serialVersionUID = 8384021804230L;

  private K key;

  private V value;

  public IntermediateRecord(K key, V value) {
    this.key = key;
    this.value = value;
  }

  public K getKey() {
    return key;
  }

  public void setKey(K key) {
    this.key = key;
  }

  public V getValue() {
    return value;
  }

  public void setValue(V value) {
    this.value = value;
  }

  @Override
  public int compareTo(IntermediateRecord<K, V> o) {
    return this.key.compareTo(o.key);
  }
}
