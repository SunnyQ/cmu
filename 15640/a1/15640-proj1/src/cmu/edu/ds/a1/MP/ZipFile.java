package cmu.edu.ds.a1.MP;

import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.BlockGZIPOutputStream;
import cmu.edu.ds.a1.IO.TransactionalFileInputStream;
import cmu.edu.ds.a1.IO.TransactionalFileOutputStream;

/**
 * The Class Zip is a basic compressor that generate gzipped file. It takes one file as input, and
 * generates the output file under a specified path.
 */

public class ZipFile implements MigratableProcess {

  /** The Constant serialVersionUID. */
  private static final long serialVersionUID = 1L;

  /** The suspending flag. */
  private volatile boolean suspending;

  /** The input stream. */
  private TransactionalFileInputStream inStream;

  /** The output stream. */
  private TransactionalFileOutputStream outStream;

  /** The arguments. */
  private String[] args;

  /**
   * Instantiates a compressor.
   * 
   * @param args
   *          the arguments
   * @throws Exception
   */
  public ZipFile(String[] args) throws Exception {
    if (args.length < 1) {
      System.out.println("usage: ZipFile <inputFile>");
      throw new Exception("Invalid Arguments");
    }
    if (!new File(args[0]).isFile()) {
      System.out.println("inputFile \"" + args[0] + "\" is not a valid input file!");
      throw new Exception("Invalid Arguments");
    }
    this.inStream = new TransactionalFileInputStream(args[0]);
    this.outStream = new TransactionalFileOutputStream(args[0] + ".gz", false);
    this.args = args;
  }

  /*
   * @see java.lang.Runnable#run()
   */
  @Override
  public void run() {
    try {
      byte[] buf = new byte[512];
      DataInputStream in = new DataInputStream(inStream);
      BlockGZIPOutputStream gzipOutStream = new BlockGZIPOutputStream(outStream);

      while (!suspending) {
        int size = in.read(buf);
        if (size == -1)
          break;
        gzipOutStream.write(buf, 0, size);
        gzipOutStream.flush();
        Thread.sleep(500);
      }

      in.close();
      gzipOutStream.close();
    } catch (IOException e) {
      System.out.println("ZipFile Error: " + e);
    } catch (InterruptedException e) {
      System.out.println("ZipFile Error: " + e);
    }
    suspending = false;
  }

  /*
   * @see cmu.edu.ds.a1.IF.MigratableProcess#suspend()
   */
  @Override
  public void suspend() {
    System.out.println("suspending...");
    suspending = true;
    while (suspending)
      ;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder("ZipFile");
    for (String s : args)
      sb.append(" " + s);
    return sb.toString();
  }

  /**
   * The main method.
   * 
   * @param args
   *          the arguments
   * @throws Exception
   */
  public static void main(String[] args) throws Exception {

    String[] tmpArgs = { "in.pdf", "out.pdf.gz" };
    ZipFile crawler = new ZipFile(tmpArgs);

    Thread t = new Thread(crawler);
    t.start();

    try {
      Thread.sleep(1000);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }

    System.out.println("1");
    crawler.suspend();
    System.out.println("2");

    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    System.out.println("3");
    t = new Thread(crawler);
    System.out.println("4");
    t.start();
    System.out.println("5");
  }

}
