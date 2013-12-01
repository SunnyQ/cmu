package MapReduce.Records;

/**
 * Interface: InputRecord.java
 * 
 * The interface used for parsing the data in a fixed length
 * 
 * @author Yuan Gu
 * 
 */
public interface InputRecord {
  public abstract void parseFromBytes(byte[] buf);
}
