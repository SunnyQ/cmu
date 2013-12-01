package Communication;

import java.util.List;

/**
 * Class: ReducerTaskPayload.java
 * 
 * The Message Payload used to communicate between master and client for reduce task
 * 
 * @author Yang Sun
 * 
 */
public class ReducerTaskPayload extends TaskPayload {

  private static final long serialVersionUID = 4939252635943129912L;

  private int reducerID;

  private List<String> inputSplits;

  private String reducerClass;

  private String outputFolder;

  public int getReducerID() {
    return reducerID;
  }

  public void setReducerID(int reducerID) {
    this.reducerID = reducerID;
  }

  public List<String> getInputSplits() {
    return inputSplits;
  }

  public void setInputSplits(List<String> inputSplits) {
    this.inputSplits = inputSplits;
  }

  public String getReducerClass() {
    return reducerClass;
  }

  public void setReducerClass(String reducerClass) {
    this.reducerClass = reducerClass;
  }

  public String getOutputFolder() {
    return outputFolder;
  }

  public void setOutputFolder(String outputFolder) {
    this.outputFolder = outputFolder;
  }

}
