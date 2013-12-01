package MapReduce.Records;

import MapReduce.Interface.Key;

/**
 * Class: IntermediateRecordSegment.java
 * 
 * The intermediate record segment
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecordSegment<K extends Key> {
  private K key;

  private long recordNum;

  public IntermediateRecordSegment(K key, long recordNum) {
    this.key = key;
    this.recordNum = recordNum;
  }

  public long getRecordNum() {
    return recordNum;
  }

  public void setRecordNum(long recordNum) {
    this.recordNum = recordNum;
  }

  public K getKey() {
    return key;
  }

  public void setKey(K key) {
    this.key = key;
  }
}
