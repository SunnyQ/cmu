package cmu.edu.ds.a1.IO;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutput;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.math.BigInteger;
import java.security.SecureRandom;

/**
 * This serves as the utility class to help serialize the object
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class SerializableWrite {

  public static String objWrite(final Serializable obj, final String filePath) {
    try {
      // build the file path
      StringBuilder path = new StringBuilder(new File(filePath).getAbsolutePath());
      path.append(File.separator);
      path.append(new BigInteger(130, new SecureRandom()).toString(32));
      path.append(".dat");
      ObjectOutput s = new ObjectOutputStream(new FileOutputStream(path.toString()));

      // write out to the stream
      s.writeObject(obj);
      s.flush();
      s.close();
      return path.toString();
    } catch (IOException e) {
      e.printStackTrace();
    }
    return null;
  }
}
