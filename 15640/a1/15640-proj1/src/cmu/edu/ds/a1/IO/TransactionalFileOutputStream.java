package cmu.edu.ds.a1.IO;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.io.Serializable;

public class TransactionalFileOutputStream extends OutputStream implements Serializable {

  private static final long serialVersionUID = 378282270137012604L;

  private long offset;

  private String filename;

  public TransactionalFileOutputStream(String filename, boolean append) {
    this.filename = filename;
    this.offset = append ? new File(filename).length() : 0L;
  }

  @Override
  public void write(int tarByte) throws IOException {
    RandomAccessFile raf = new RandomAccessFile(filename, "rws");
    raf.seek(offset++);
    raf.write(tarByte);
    raf.close();
  }

  @Override
  public void write(byte[] b) throws IOException {
    RandomAccessFile raf = new RandomAccessFile(filename, "rws");
    raf.seek(offset);
    raf.write(b);
    offset += b.length;
    raf.close();
  }

  @Override
  public void write(byte[] b, int offset, int len) throws IOException {
    RandomAccessFile raf = new RandomAccessFile(filename, "rws");
    raf.seek(this.offset);
    raf.write(b, offset, len);
    this.offset += len;
    raf.close();
  }

}