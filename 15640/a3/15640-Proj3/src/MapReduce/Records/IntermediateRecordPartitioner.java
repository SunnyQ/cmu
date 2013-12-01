package MapReduce.Records;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.HashMap;
import java.util.Map;

import MapReduce.Interface.Key;
import MapReduce.Interface.Partitioner;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateRecordPartitioner.java
 * 
 * This serves as a tool to partition the intermediate records according to the number of reducers
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecordPartitioner<K extends Key, V extends Value> {

  private int bufferSize = 10;

  private String tmpDir = null;

  private Partitioner<K, V> partitioner;

  private int partitionNum = 1;

  private Map<Integer, ObjectOutputStream> partitionMap = new HashMap<Integer, ObjectOutputStream>();

  public int getPartitionNum() {
    return partitionNum;
  }

  public void setPartitionNum(int partitionNum) {
    this.partitionNum = partitionNum;
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

  public Partitioner<K, V> getPartitioner() {
    return partitioner;
  }

  public void setPartitioner(Partitioner<K, V> partitioner) {
    this.partitioner = partitioner;
  }

  /**
   * Partition the result for the reducer
   * 
   * @param filePath
   * @return
   * @throws FileNotFoundException
   * @throws IOException
   */
  public Map<Integer, String> partition(String filePath) throws FileNotFoundException, IOException {

    Map<Integer, String> partitionPathMap = new HashMap<Integer, String>();

    IntermediateRecordIterator<K, V> reader = new IntermediateRecordIterator<K, V>(filePath);
    IntermediateRecord<K, V> record = null;

    this.partitionMap.clear();

    while (reader.hasNext()) {
      record = reader.next();
      int partitionID = this.partitioner.getPartitionID(record.getKey(), record.getValue(),
              this.partitionNum);
      if (!this.partitionMap.containsKey(partitionID)) {
        String partitionPath = this.tmpDir + File.separator + "partition_" + partitionID;
        ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(partitionPath));
        this.partitionMap.put(partitionID, out);
        partitionPathMap.put(partitionID, partitionPath);
      }
      ObjectOutputStream out = this.partitionMap.get(partitionID);
      out.writeObject(record);
    }

    reader.close();

    for (ObjectOutputStream out : this.partitionMap.values()) {
      out.flush();
      out.close();
    }

    return partitionPathMap;
  }

  // public static void main(String[] args) {
  //
  // String path = "/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/tmp_11_1";
  // try {
  // IntermediateRecordPartitioner<StringWritable, StringWritable> reader = new
  // IntermediateRecordPartitioner<StringWritable, StringWritable>();
  // reader.setBufferSize(100);
  // reader.setPartitionNum(4);
  // reader.setTmpDir("/Users/htcbug/Documents/Homework/15640/hw3/map-tmp");
  // reader.setPartitioner(new HashPartitioner<StringWritable, StringWritable>());
  // reader.partition(path);
  // } catch (Exception e) {
  // e.printStackTrace();
  // }
  // }

}
