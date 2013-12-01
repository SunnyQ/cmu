package Communication;

import java.util.Map;

/**
 * Class: MapperTaskPayload.java
 * 
 * The Message Payload used to communicate between master and client for map task
 * 
 * @author Yang Sun
 * 
 */
public class MapperTaskPayload extends TaskPayload {

  private static final long serialVersionUID = 4939252635943129912L;

  private int mapperID;

  private String mapperClass;

  private String reducerClass;

  private String inputFile;

  private String outputFolder;

  private Map<Integer, String> resultPartitions;

  public int getMapperID() {
    return mapperID;
  }

  public void setMapperID(int mapperID) {
    this.mapperID = mapperID;
  }

  public String getMapperClass() {
    return mapperClass;
  }

  public void setMapperClass(String mapperClass) {
    this.mapperClass = mapperClass;
  }

  public Map<Integer, String> getResultPartitions() {
    return resultPartitions;
  }

  public void setResultPartitions(Map<Integer, String> resultPartitions) {
    this.resultPartitions = resultPartitions;
  }

  public String getReducerClass() {
    return reducerClass;
  }

  public void setReducerClass(String reducerClass) {
    this.reducerClass = reducerClass;
  }

  public String getInputFile() {
    return inputFile;
  }

  public void setInputFile(String inputFile) {
    this.inputFile = inputFile;
  }

  public String getOutputFolder() {
    return outputFolder;
  }

  public void setOutputFolder(String outputFolder) {
    this.outputFolder = outputFolder;
  }

}
