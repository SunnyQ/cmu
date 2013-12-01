package MapReduce.Records;

/**
 * Interface: OutputRecord.java
 * 
 * The type of the output record which can be implemented and extended
 * 
 * @author Yuan Gu
 * 
 */
public interface OutputRecord {
  public abstract byte[] toBytes(int len);
}
