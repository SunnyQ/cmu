package cmu.edu.ds.a1.IO;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.io.Serializable;
import java.io.StringReader;

public class TransactionalFileInputStream extends InputStream implements Serializable {

  private static final long serialVersionUID = 8525596152925121720L;

  private long offset;

  private String filename;

  public TransactionalFileInputStream(String filename) {
    this.filename = filename;
    this.offset = 0L;
  }

  @Override
  public int read() throws IOException {
    RandomAccessFile raf = new RandomAccessFile(filename, "rws");
    raf.seek(offset);
    int nextByte = raf.read();
    if (nextByte != -1)
      offset++;
    raf.close();
    return nextByte;
  }

//  @Override
//  public int read(byte[] b) throws IOException {
//    RandomAccessFile raf = new RandomAccessFile(filename, "rws");
//    raf.seek(offset);
//    int readSize = raf.read(b);
//    if (readSize > 0)
//      offset += readSize;
//    raf.close();
//    return readSize;
//  }

  public static void main(String[] args) throws Exception {

    String s = "ABCDE\nFDAFAS\nfdalfksadjflsalkfjas";
    StringReader sr = null;
    BufferedReader br = null;

    try {

      sr = new StringReader(s);

      // create new buffered reader
      br = new BufferedReader(sr);

      // reads and prints BufferedReader
      System.out.println(br.readLine());
      br.mark(0);
      System.out.println("mark() invoked");
      System.out.println(br.readLine());

      // reset() repositioned the stream to the mark
      br.reset();
      System.out.println("reset() invoked");
      System.out.println(br.readLine());

    } catch (Exception e) {

      // exception occurred.
      e.printStackTrace();
    } finally {

      // releases any system resources associated with the stream
      if (sr != null)
        sr.close();
      if (br != null)
        br.close();
    }
  }
}
