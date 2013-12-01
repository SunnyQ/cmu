package MapReduce.Records;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import MapReduce.Interface.Key;
import MapReduce.Interface.Value;

/**
 * Class: IntermediateRecordMergeSorter.java
 * 
 * This serves as a tool to merge sort the intermediate records output from the mapper
 * 
 * @author Yuan Gu
 * 
 */
public class IntermediateRecordMergeSorter<K extends Key, V extends Value> {

  private int bufferSize = 1000;

  private String tmpDir;

  /**
   * Returns record number in each key.
   * 
   * @param filePath1
   * @param filePath2
   * @param mergedFilePath
   * @return
   * @throws FileNotFoundException
   * @throws IOException
   */
  private List<IntermediateRecordSegment<K>> mergeTwoTmpFiles(String filePath1, String filePath2,
          String mergedFilePath) throws FileNotFoundException, IOException {

    System.out.println("Merging " + filePath1 + " and " + filePath2 + " to " + mergedFilePath);

    List<IntermediateRecordSegment<K>> recordNums = new LinkedList<IntermediateRecordSegment<K>>();
    long recordNumCurrentKey = 0;
    int numRecord = 0;

    IntermediateRecordIterator<K, V> itr1 = new IntermediateRecordIterator<K, V>(filePath1,
            this.bufferSize);
    IntermediateRecordIterator<K, V> itr2 = new IntermediateRecordIterator<K, V>(filePath2,
            this.bufferSize);

    ObjectOutputStream out = new ObjectOutputStream(new BufferedOutputStream(new FileOutputStream(
            mergedFilePath)));

    IntermediateRecord<K, V> record1 = itr1.hasNext() ? itr1.next() : null;
    IntermediateRecord<K, V> record2 = itr2.hasNext() ? itr2.next() : null;
    IntermediateRecord<K, V> currRecord = null;
    IntermediateRecord<K, V> lastRecord = null;

    while (record1 != null || record2 != null) {

      if (record1 != null && (record2 == null || record1.compareTo(record2) <= 0)) {
        currRecord = record1;
        record1 = itr1.hasNext() ? itr1.next() : null;
      } else {
        currRecord = record2;
        record2 = itr2.hasNext() ? itr2.next() : null;
      }

      out.writeObject(currRecord);
      numRecord++;

      if (lastRecord == null) {
        recordNumCurrentKey++;
        lastRecord = currRecord;
      } else if (currRecord.compareTo(lastRecord) == 0) {
        recordNumCurrentKey++;
      } else {
        recordNums.add(new IntermediateRecordSegment<K>(lastRecord.getKey(), recordNumCurrentKey));
        recordNumCurrentKey = 1;
        lastRecord = currRecord;
      }

    }

    /* add the last number to list */
    if (lastRecord != null)
      recordNums.add(new IntermediateRecordSegment<K>(lastRecord.getKey(), recordNumCurrentKey));

    out.flush();

    out.close();
    itr1.close();
    itr2.close();

    System.out.println("Finished with recordNum " + numRecord);
    return recordNums;

  }

  private String getFilePath(int phaseNum, int tmpFileSeq) {
    return this.tmpDir + File.separator + "tmp_" + phaseNum + "_" + tmpFileSeq;
  }

  /**
   * Sort the split files
   * 
   * @param splitPaths
   * @param sortedPath
   * @return a list of intermediate record segments
   * @throws FileNotFoundException
   * @throws IOException
   */
  public List<IntermediateRecordSegment<K>> sortSplits(List<String> splitPaths, String sortedPath)
          throws FileNotFoundException, IOException {

    System.out.println("Input: " + splitPaths);

    List<IntermediateRecordSegment<K>> recordNums = null;
    List<String> toMergePaths = new ArrayList<String>(splitPaths);

    int maxSeq = splitPaths.size();
    int lastSeq = -1;
    int phaseNum = 1;

    if (maxSeq == 1) {
      IntermediateRecordIterator<K, V> itr = new IntermediateRecordIterator<K, V>(
              splitPaths.get(0), this.bufferSize);
      if (itr.hasNext()) {
        recordNums = new LinkedList<IntermediateRecordSegment<K>>();
        recordNums.add(new IntermediateRecordSegment<K>(itr.next().getKey(), Long.MAX_VALUE));
      }
      itr.close();
    } else {
      while (maxSeq > 1) {
        List<String> mergedPaths = new ArrayList<String>();
        lastSeq = -1;

        if (maxSeq % 2 != 0) {
          lastSeq = maxSeq;
          maxSeq = maxSeq - 1;
        }

        System.out.println("maxSeq " + maxSeq + " lastSeq " + lastSeq);

        for (int i = 1; i <= maxSeq / 2; i++) {
          recordNums = mergeTwoTmpFiles(toMergePaths.get(i - 1),
                  toMergePaths.get(i + maxSeq / 2 - 1), getFilePath(phaseNum + 1, i));
          mergedPaths.add(getFilePath(phaseNum + 1, i));
        }

        maxSeq = maxSeq / 2;

        /* remains one file to merge */
        if (lastSeq != -1) {
          util.FileOperation.renameFile(toMergePaths.get(lastSeq - 1),
                  getFilePath(phaseNum + 1, maxSeq + 1), true);
          mergedPaths.add(getFilePath(phaseNum + 1, maxSeq + 1));
          maxSeq++;
        }

        phaseNum++;
        toMergePaths = mergedPaths;
      }
    }

    util.FileOperation.renameFile(toMergePaths.get(maxSeq - 1), sortedPath, true);
    return recordNums;
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

  // public static void main(String args[]) throws FileNotFoundException, IOException {
  //
  // IntermediateRecordMergeSorter<StringWritable, StringWritable> recordMergeSorter = new
  // IntermediateRecordMergeSorter<StringWritable, StringWritable>();
  // recordMergeSorter.setBufferSize(1);
  // recordMergeSorter.setTmpDir("/Users/htcbug/Documents/Homework/15640/hw3/reduce-tmp");
  //
  // List<String> paths = new LinkedList<String>();
  // paths.add("/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/partition_0");
  // paths.add("/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/partition_1");
  // paths.add("/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/partition_2");
  // paths.add("/Users/htcbug/Documents/Homework/15640/hw3/map-tmp/partition_3");
  // recordMergeSorter.sortSplits(paths,
  // "/Users/htcbug/Documents/Homework/15640/hw3/reduce-tmp/partition_test");
  // }
}
