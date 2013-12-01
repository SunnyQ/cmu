package cmu.edu.ds.a1.MP;

import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.PrintStream;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.TransactionalFileInputStream;
import cmu.edu.ds.a1.IO.TransactionalFileOutputStream;

public class GrepProcess implements MigratableProcess {
  /**
   * 
   */
  private static final long serialVersionUID = -2909800760440217455L;

  /**
   * 
   */
  private TransactionalFileInputStream inFile;

  private TransactionalFileOutputStream outFile;

  private String query;

  private String[] args;

  private volatile boolean suspending;

  public GrepProcess(String args[]) throws Exception {
    if (args.length != 3) {
      System.out.println("usage: GrepProcess <queryString> <inputFile> <outputFile>");
      throw new Exception("Invalid Arguments");
    }

    this.args = args;
    query = args[0];
    inFile = new TransactionalFileInputStream(args[1]);
    outFile = new TransactionalFileOutputStream(args[2], false);
  }

  @Override
  public void run() {
    PrintStream out = new PrintStream(outFile);
    DataInputStream in = new DataInputStream(inFile);

    try {
      while (!suspending) {
        @SuppressWarnings("deprecation")
        String line = in.readLine(); // YANG: make aware of this, deprecated

        if (line == null)
          break;

        if (line.contains(query)) {
          out.println(line);
        }

        // Make grep take longer so that we don't require extremely
        // large files for interesting results
        try {
          Thread.sleep(1000);
        } catch (InterruptedException e) {
          // ignore it
        }
      }
    } catch (EOFException e) {
      System.out.println("End of line " + e);
    } catch (IOException e) {
      System.out.println("GrepProcess: Error: " + e);
    }

    suspending = false;
  }

  @Override
  public void suspend() {
    suspending = true;
    System.out.println("received suspending");
    while (suspending)
      ;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder("GrepProcess");
    for (String s : args)
      sb.append(" " + s);
    return sb.toString();
  }
}