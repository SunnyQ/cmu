package Communication;

/**
 * Class: TaskPayload.java
 * 
 * The abstract class that is intended to be extended by MapperTaskPayload and ReducerTaskPayload
 * 
 * @author Yang Sun
 * 
 */
public abstract class TaskPayload extends Message.MessagePayload {
  private static final long serialVersionUID = 7507222233008008949L;

  private long taskID;

  private String jarFile;

  private String clientID;

  private String issuer;

  public long getTaskID() {
    return taskID;
  }

  public void setTaskID(long taskID) {
    this.taskID = taskID;
  }

  public String getJarFile() {
    return jarFile;
  }

  public void setJarFile(String jarFile) {
    this.jarFile = jarFile;
  }

  public String getClientID() {
    return clientID;
  }

  public void setClientID(String clientID) {
    this.clientID = clientID;
  }

  public String getIssuer() {
    return issuer;
  }

  public void setIssuer(String issuer) {
    this.issuer = issuer;
  }

}
