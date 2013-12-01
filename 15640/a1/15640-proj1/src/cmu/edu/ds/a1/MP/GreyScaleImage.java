package cmu.edu.ds.a1.MP;

import java.io.FileNotFoundException;
import java.io.IOException;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.TransactionalFileInputStream;
import cmu.edu.ds.a1.IO.TransactionalFileOutputStream;

public class GreyScaleImage implements MigratableProcess {

  /**
   * 
   */
  private static final long serialVersionUID = -8292448172674006456L;

  private volatile boolean suspended;

  private String inputPath;

  private String outputPath;

  private TransactionalFileInputStream serIn;

  private TransactionalFileOutputStream serOut;

  public GreyScaleImage(String[] args) throws FileNotFoundException {
    // TODO Auto-generated constructor stub
    this.suspended = false;
    this.inputPath = args[0];
    this.outputPath = args[1];
    this.serIn = new TransactionalFileInputStream(this.inputPath);
    this.serOut = new TransactionalFileOutputStream(this.outputPath, true);
  }

  @Override
  public void run() {
    // TODO Yang: dummy part
    try {
      serIn.read();
      serOut.write(null);
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  @Override
  public void suspend() {
    // TODO Auto-generated method stub
    this.suspended = true;
    while (this.suspended)
      ;
  }

  @Override
  public String toString() {
    // TODO Auto-generated method stub
    return super.toString();
  }

}
