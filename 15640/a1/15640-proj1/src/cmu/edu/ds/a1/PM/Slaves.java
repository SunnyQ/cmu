package cmu.edu.ds.a1.PM;

import java.util.ArrayList;

import cmu.edu.ds.a1.Thread.M2SThread;

/**
 * This serves as a wrapper class for AraryList that maintains all the slave threads
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class Slaves extends ArrayList<M2SThread> {

  private static final long serialVersionUID = -2199238192592655497L;

  /**
   * traverse each slave and collect for the average load level; remove the inactive slaves during
   * the traversal
   * 
   * @param masterLevel
   *          the current load level of the master node
   * @return an int which represents the current average load level
   */
  public int getLoadLevel(int masterLevel) {
    int totalLevel = masterLevel;
    int i = 0;
    while (i < super.size()) {
      if (!get(i).isAlive())
        remove(i);
      else
        totalLevel += get(i++).getNumProcesses();
    }
    return (int) Math.ceil(totalLevel / (size() + 1.0));
  }

  public void refreshStatus() {
    int i = 0;
    while (i < super.size()) {
      if (!get(i).isAlive())
        remove(i);
      else
        i++;
    }
  }
}
