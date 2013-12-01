package cmu.edu.ds.a1.PM;

import java.util.ArrayList;

import cmu.edu.ds.a1.IF.MigratableProcess;

/**
 * This serves as a wrapper class for ArrayList that overrides the initial size method in order to
 * do a dead thread collection while it is being invoked.
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class Processes extends ArrayList<ThreadWrapper<MigratableProcess>> {

  private static final long serialVersionUID = -7664995909224665317L;

  @Override
  public int size() {
    refreshStatus();
    return super.size();
  }

  public void refreshStatus() {
    int i = 0;
    while (i < super.size()) {
      // remove the process if it is already done
      if (!get(i).isAlive())
        System.out.println("Process \"" + remove(i).getTarget().toString() + "\" was terminated");
      else
        i++;
    }
  }

}
