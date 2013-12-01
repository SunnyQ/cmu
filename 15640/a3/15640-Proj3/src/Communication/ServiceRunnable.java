package Communication;

import java.io.IOException;

/**
 * Interface: ServiceRunnable.java
 * 
 * The serves as a marker for services running at the background
 * 
 * @author Yang Sun
 * 
 */
public interface ServiceRunnable extends Runnable {

  void shutdown() throws IOException;

  boolean isAlive();
}
