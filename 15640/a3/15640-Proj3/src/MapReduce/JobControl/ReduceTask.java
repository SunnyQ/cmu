package MapReduce.JobControl;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.List;

import Communication.Message;
import MapReduce.IO.OutputRecordWriter;
import MapReduce.Interface.Key;
import MapReduce.Interface.Reducer;
import MapReduce.Interface.Value;
import MapReduce.Records.IntermediateRecordIterator;
import MapReduce.Records.IntermediateRecordMergeSorter;
import MapReduce.Records.IntermediateRecordSegment;
import MapReduce.Records.IntermediateValueIterator;

/**
 * Class: ReduceTask.java
 * 
 * The reduce task driver, runnable from any client task
 * 
 * @author Yuan Gu
 * 
 */
public class ReduceTask<K extends Key, V extends Value> implements Runnable {

  private List<String> splitPaths;

  private String tmpDir;

  private int outputOffset = 0;

  private int bufferSize = 1000;

  private String signalAddr;

  private int signalPort;

  private Message responseMsg;

  private String outputIdentifier;

  private String outputFolder;

  private Class<? extends Reducer<K, V>> reducerClass;

  public int getOutputOffset() {
    return outputOffset;
  }

  public void setOutputOffset(int outputOffset) {
    this.outputOffset = outputOffset;
  }

  public String getTmpDir() {
    return tmpDir;
  }

  public void setTmpDir(String tmpDir) {
    this.tmpDir = tmpDir;
  }

  public int getBufferSize() {
    return bufferSize;
  }

  public void setBufferSize(int bufferSize) {
    this.bufferSize = bufferSize;
  }

  public List<String> getSplitPaths() {
    return splitPaths;
  }

  public void setSplitPaths(List<String> splitPaths) {
    this.splitPaths = splitPaths;
  }

  /**
   * Merge sort the intermediate result
   * 
   * @param mergedPath
   * @return list of intermediate record segments
   * @throws FileNotFoundException
   * @throws IOException
   */
  private List<IntermediateRecordSegment<K>> mergeSplits(String mergedPath)
          throws FileNotFoundException, IOException {

    IntermediateRecordMergeSorter<K, V> recordMergeSorter = new IntermediateRecordMergeSorter<K, V>();
    recordMergeSorter.setBufferSize(this.bufferSize);
    recordMergeSorter.setTmpDir(this.tmpDir);
    return recordMergeSorter.sortSplits(this.splitPaths, mergedPath);
  }

  @Override
  public void run() {

    System.out.println("Started reducing");
    try {
      /* Prepare the data and reducer */
      String path = this.tmpDir + File.separator + "records.sorted";
      List<IntermediateRecordSegment<K>> recordSegments = mergeSplits(path);
      Reducer<K, V> reducer = this.reducerClass.newInstance();

      /* Prepare the output file */
      new File(outputFolder).mkdirs();
      String outputFile = outputFolder + File.separator + outputIdentifier;
      OutputRecordWriter outputRecordWriter = new OutputRecordWriter(outputFile, this.outputOffset,
              reducer.getRecordLength());

      IntermediateRecordIterator<K, V> reader = new IntermediateRecordIterator<K, V>(path,
              this.bufferSize);

      /* Reduce and Write out the result */
      for (IntermediateRecordSegment<K> segment : recordSegments) {
        IntermediateValueIterator<V> intermediateValueIterator = new IntermediateValueIterator<V>(
                reader, segment.getRecordNum());

        reducer.Reduce(segment.getKey(), intermediateValueIterator, outputRecordWriter);
      }

      reader.close();
      outputRecordWriter.close();

      /* Notify the master controller for finishing the task */
      Message msg = new Message(Message.TYPE.REDUCER, responseMsg.getPayload());
      Message.sendRequest(signalAddr, signalPort, 5000, msg);

    } catch (Exception e) {
      sendException();
    }
  }

  /**
   * Notify the master for any exceptions happened while running the user defined map task.
   */
  private void sendException() {
    try {
      Message exceptionMsg = new Message(Message.TYPE.EXCEPTION, responseMsg.getPayload());
      Message.sendRequest(signalAddr, signalPort, 5000, exceptionMsg);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public Class<? extends Reducer<K, V>> getReducerClass() {
    return reducerClass;
  }

  public void setReducerClass(Class<? extends Reducer<K, V>> reducerClass) {
    this.reducerClass = reducerClass;
  }

  public Message getResponseMsg() {
    return responseMsg;
  }

  public void setResponseMsg(Message incomingMsg) {
    responseMsg = incomingMsg;
  }

  public String getOutputIdentifier() {
    return outputIdentifier;
  }

  public void setOutputIdentifier(String outputIdentifier) {
    this.outputIdentifier = outputIdentifier;
  }

  // public static void main(String[] args) throws IOException, InstantiationException,
  // IllegalAccessException {
  //
  // ReduceTask<StringWritable, StringWritable> reduceTask = new ReduceTask<StringWritable,
  // StringWritable>();
  //
  // List<String> splitPaths = new ArrayList<String>();
  // splitPaths.add("tmp/client_1/1365813735873/map/partition_0");
  // splitPaths.add("tmp/client_2/1365813735873/map/partition_0");
  // // splitPaths.add("map-tmp/partition_2");
  // // splitPaths.add("map-tmp/partition_3");
  //
  // String tmpDir = "reduce-tmp";
  // if (!util.FileOperation.createDir(tmpDir)) {
  // System.out.println("Fail to create tmpDir");
  // }
  // reduceTask.setTmpDir(tmpDir);
  // reduceTask.setSplitPaths(splitPaths);
  // reduceTask.setBufferSize(1000);
  // // reduceTask.setReducerClass(StringInputReducer.class);
  // reduceTask.setOutputOffset(0);
  // reduceTask.run();
  // }

  public String getSignalAddr() {
    return signalAddr;
  }

  public void setSignalAddr(String signalAddr) {
    this.signalAddr = signalAddr;
  }

  public int getSignalPort() {
    return signalPort;
  }

  public void setSignalPort(int signalPort) {
    this.signalPort = signalPort;
  }

  public String getOutputFolder() {
    return outputFolder;
  }

  public void setOutputFolder(String outputFolder) {
    this.outputFolder = outputFolder;
  }

}
