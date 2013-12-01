package MapReduce.Records;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateRecordCollector.java
 * 
 * The collector that collects all intermediate records
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecordCollector<K extends Key, V extends Value> {

  private String tmpDir;

  private int tmpFileSeq = 1;

  private int phaseNum = 1;

  private int bufferSize = 10;

  private List<String> splitPaths = new ArrayList<String>();

  private List<IntermediateRecord<K, V>> recordBuffer = new LinkedList<IntermediateRecord<K, V>>();

  private String getFilePath(int phaseNum, int tmpFileSeq) {
    return this.tmpDir + File.separator + "split_" + tmpFileSeq;
  }

  /**
   * Dump the result to split files
   * 
   * @throws IOException
   */
  private void dumpToSplit() throws IOException {

    int recordNum = 0;

    Collections.sort(recordBuffer);
    String splitPath = getFilePath(phaseNum, tmpFileSeq);
    System.out.println("dumpToTmpFile called, with File: " + splitPath);
    ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(splitPath));
    for (IntermediateRecord<K, V> record : recordBuffer) {
      recordNum++;
      out.writeObject(record);
      out.flush();
    }

    out.close();
    recordBuffer.clear();
    this.splitPaths.add(splitPath);
    tmpFileSeq++;
    System.out.println("dumpToTmpFile finished with " + recordNum + " records dumped.");
  }

  /**
   * Merge sort the temporary files
   * 
   * @param sortedPath
   * @throws IOException
   * @throws ClassNotFoundException
   */
  private void sortTmpFiles(String sortedPath) throws IOException, ClassNotFoundException {
    IntermediateRecordMergeSorter<K, V> recordMergeSorter = new IntermediateRecordMergeSorter<K, V>();
    recordMergeSorter.setBufferSize(this.bufferSize);
    recordMergeSorter.setTmpDir(this.tmpDir);
    recordMergeSorter.sortSplits(this.splitPaths, sortedPath);
  }

  /**
   * Task driver
   * 
   * @return the output temporary file path
   * @throws IOException
   * @throws ClassNotFoundException
   */
  public String finish() throws IOException, ClassNotFoundException {
    dumpToSplit();
    String path = this.tmpDir + File.separator + "records.sorted";
    sortTmpFiles(path);
    return path;
  }

  /**
   * Collect the result and dump to the split file
   * 
   * @param key
   * @param value
   * @throws IOException
   */
  public void collect(K key, V value) throws IOException {
    this.recordBuffer.add(new IntermediateRecord<K, V>(key, value));
    if (this.recordBuffer.size() >= this.bufferSize)
      dumpToSplit();
  }

  public int getBufferSize() {
    return bufferSize;
  }

  public void setBufferSize(int bufferSize) {
    this.bufferSize = bufferSize;
  }

  public String getTmpDir() {
    return tmpDir;
  }

  public void setTmpDir(String tmpDir) {
    this.tmpDir = tmpDir;
  }

}
