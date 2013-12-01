package MapReduce.Implementation;

import java.io.UnsupportedEncodingException;

import MapReduce.Records.InputRecord;

/**
 * Class: StringInputRecord.java
 * 
 * The type of input the mapper or reducer can use
 * 
 * @author Yuan Gu
 * 
 */
public class StringInputRecord implements InputRecord {

  private String value = null;

  @Override
  public String toString() {
    return value;
  }

  @Override
  public void parseFromBytes(byte[] buf) {
    int len = buf[0];
    byte[] tmpBuf = new byte[len];
    for (int i = 0; i < len; i++)
      tmpBuf[i] = buf[i + 1];
    try {
      value = new String(tmpBuf, "UTF-8");
    } catch (UnsupportedEncodingException e) {
    }
  }
}
