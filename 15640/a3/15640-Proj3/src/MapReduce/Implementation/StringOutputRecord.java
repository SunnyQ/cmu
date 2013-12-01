package MapReduce.Implementation;

import MapReduce.Records.OutputRecord;

/**
 * Class: StringOutputRecord.java
 * 
 * The type of output the mapper or reducer can use
 * 
 * @author Yuan Gu
 * 
 */
public class StringOutputRecord implements OutputRecord {

  private String value = null;

  public String getValue() {
    return value;
  }

  public void setValue(String value) {
    this.value = value;
  }

  @Override
  public String toString() {
    return value;
  }

  @Override
  public byte[] toBytes(int len) {
    byte[] buf = new byte[len];
    byte[] valueBuf = value.getBytes();
    buf[0] = new Integer(valueBuf.length).byteValue();
    for (int i = 0; i < Math.min(buf.length, value.length()); i++)
      buf[i + 1] = valueBuf[i];
    return buf;

  }
}
