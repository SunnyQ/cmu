package cmu.edu.ds.a1.PM;

import cmu.edu.ds.a1.IF.MigratableProcess;

/**
 * This serves as a wrapper of Thread class that contains one extra reference to the target runnable
 * object
 * 
 * @author Yang Sun, Yuan Gu
 * 
 * @param <T extends MigratableProcess>target runnable instance
 */
public class ThreadWrapper<T extends MigratableProcess> extends Thread {

  private T target;

  public ThreadWrapper(T target) {
    super(target);
    this.target = target;
  }

  public T getTarget() {
    return target;
  }

  public void setTarget(T target) {
    this.target = target;
  }

}
