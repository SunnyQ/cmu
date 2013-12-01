package MapReduce.JobControl;

import java.io.IOException;
import java.util.Map;

import Communication.MapperTaskPayload;
import Communication.Message;
import MapReduce.IO.InputRecordReader;
import MapReduce.Interface.Key;
import MapReduce.Interface.Mapper;
import MapReduce.Interface.Partitioner;
import MapReduce.Interface.Value;
import MapReduce.Records.InputRecord;
import MapReduce.Records.IntermediateRecordCollector;
import MapReduce.Records.IntermediateRecordPartitioner;

/**
 * Class: MapTask.java
 * 
 * The map task driver, runnable from any client task
 * 
 * @author Yuan Gu
 * 
 */
public class MapTask<T extends InputRecord, K extends Key, V extends Value> implements Runnable {

  private long fileOffset;

  private Class<T> inputRecordClass;

  private String tmpDir;

  private long numRecordsRead;

  private int bufferSize = 1000;

  private Class<? extends Mapper<T, K, V>> mapperClass;

  @SuppressWarnings("rawtypes")
  private Class<? extends Partitioner> partitionerClass;

  private String signalAddr;

  private int signalPort;

  private Message responseMsg;

  private String inputFile;

  @SuppressWarnings("unchecked")
  @Override
  public void run() {

    InputRecordReader<T> inputRecordReader = null;
    IntermediateRecordCollector<K, V> intermediateRecordCollector = null;
    Mapper<T, K, V> mapper = null;
    T record = null;

    String intermediateResultPath = null;

    System.out.println("Started mapping");
    try {

      mapper = this.mapperClass.newInstance();

      /* Read the input records */
      inputRecordReader = new InputRecordReader<T>(inputFile, fileOffset, inputRecordClass,
              mapper.getRecordLength());

      /* Prepare the intermediate record collector */
      intermediateRecordCollector = new IntermediateRecordCollector<K, V>();
      intermediateRecordCollector.setTmpDir(this.tmpDir);
      intermediateRecordCollector.setBufferSize(this.bufferSize);

      /* Run map task defined by the user */
      for (int i = 0; i < this.numRecordsRead; i++) {
        if ((record = inputRecordReader.getNextRecord()) != null)
          mapper.Map(record, intermediateRecordCollector);
        else
          break;
      }
      intermediateResultPath = intermediateRecordCollector.finish();

      /* Partition the output result */
      IntermediateRecordPartitioner<K, V> partitioner = new IntermediateRecordPartitioner<K, V>();
      partitioner.setBufferSize(this.bufferSize);
      partitioner.setPartitionNum(mapper.getNumReducer());
      partitioner.setTmpDir(this.tmpDir);
      partitioner.setPartitioner(this.partitionerClass.newInstance());
      Map<Integer, String> res = partitioner.partition(intermediateResultPath);

      /* Send back the result to the master controller */
      ((MapperTaskPayload) responseMsg.getPayload()).setResultPartitions(res);
      Message msg = new Message(Message.TYPE.MAPPER, responseMsg.getPayload());
      Message.sendRequest(signalAddr, signalPort, 5000, msg);

    } catch (Exception e) {
      sendException();
    } finally {
      if (inputRecordReader != null)
        try {
          inputRecordReader.close();
        } catch (IOException e) {
        }
    }

    System.out.println("Finished mapping " + intermediateResultPath);
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

  public long getFileOffset() {
    return fileOffset;
  }

  public void setFileOffset(long offset) {
    this.fileOffset = offset;
  }

  public Class<T> getInputRecordClass() {
    return inputRecordClass;
  }

  public void setInputRecordClass(Class<T> inputRecordClass) {
    this.inputRecordClass = inputRecordClass;
  }

  public String getTmpDir() {
    return tmpDir;
  }

  public void setTmpDir(String tmpDir) {
    this.tmpDir = tmpDir;
  }

  public Class<? extends Mapper<T, K, V>> getMapperClass() {
    return mapperClass;
  }

  public void setMapperClass(Class<? extends Mapper<T, K, V>> mapperClass) {
    this.mapperClass = mapperClass;
  }

  public int getBufferSize() {
    return bufferSize;
  }

  public void setBufferSize(int bufferSize) {
    this.bufferSize = bufferSize;
  }

  public long getNumRecordsRead() {
    return numRecordsRead;
  }

  public void setNumRecordsRead(long numRecordsRead) {
    this.numRecordsRead = numRecordsRead;
  }

  @SuppressWarnings("rawtypes")
  public Class<? extends Partitioner> getPartitionerClass() {
    return partitionerClass;
  }

  @SuppressWarnings("rawtypes")
  public void setPartitionerClass(Class<? extends Partitioner> partitionerClass) {
    this.partitionerClass = partitionerClass;
  }

  public Message getResponseMsg() {
    return responseMsg;
  }

  public void setResponseMsg(Message responseMsg) {
    this.responseMsg = responseMsg;
  }

  // public static void main(String[] args) throws IOException, InstantiationException,
  // IllegalAccessException {
  //
  // MapTask<StringInputRecord, StringWritable, StringWritable> mapTask = new
  // MapTask<StringInputRecord, StringWritable, StringWritable>();
  // mapTask.setFileOffset(8000);
  // mapTask.setNumRecordsRead(500);
  // mapTask.setInputRecordClass(StringInputRecord.class);
  // // mapTask.setMapperClass(StringInputMapper.class);
  // mapTask.setPartitionerClass(HashPartitioner.class);
  //
  // String tmpDir = "map-tmp";
  // if (!util.FileOperation.createDir(tmpDir)) {
  // System.out.println("Fail to create tmpDir");
  // }
  // mapTask.setTmpDir(tmpDir);
  // mapTask.run();
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

  public String getInputFile() {
    return inputFile;
  }

  public void setInputFile(String inputFile) {
    this.inputFile = inputFile;
  }

}
