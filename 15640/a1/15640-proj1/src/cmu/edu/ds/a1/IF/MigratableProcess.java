package cmu.edu.ds.a1.IF;

import java.io.Serializable;

public interface MigratableProcess extends Runnable, Serializable {
  void suspend();
}
