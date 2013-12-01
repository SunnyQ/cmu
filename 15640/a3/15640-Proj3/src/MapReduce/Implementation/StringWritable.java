package MapReduce.Implementation;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: StringWritable.java
 * 
 * The type that the mapper or reducer can use
 * 
 * @author Yuan Gu
 * 
 */
public class StringWritable implements Key, Value {

  private static final long serialVersionUID = 1L;

  String str = null;

  public StringWritable(String str) {
    this.str = str;
  }

  @Override
  public String toString() {
    return this.str;
  }

  @Override
  public int compareTo(Key arg0) {
    if (arg0 instanceof StringWritable) {
      return this.str.compareTo(((StringWritable) arg0).str);
    }
    return this.str.hashCode() - arg0.hashCode();
  }

  @Override
  public int hashCode() {
    return this.str.hashCode();
  }

}
