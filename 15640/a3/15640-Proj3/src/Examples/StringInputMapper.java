package Examples;

import java.io.IOException;

import MapReduce.Implementation.StringInputRecord;
import MapReduce.Implementation.StringWritable;
import MapReduce.Interface.Mapper;
import MapReduce.Records.IntermediateRecordCollector;

/**
 * Class: StringInputMapper.java
 * 
 * A mapper example to output all input records
 * 
 * @author Yuan Gu
 * 
 */
public class StringInputMapper implements Mapper<StringInputRecord, StringWritable, StringWritable> {

  @Override
  public void Map(StringInputRecord record,
          IntermediateRecordCollector<StringWritable, StringWritable> collector) {
    try {
      int key = Integer.parseInt(record.toString());
      collector.collect(new StringWritable("" + key % getNumReducer()),
              new StringWritable(record.toString()));
    } catch (IOException e) {
    }
  }

  @Override
  public int getNumMapper() {
    return 2;
  }

  @Override
  public int getNumReducer() {
    return 1;
  }

  @Override
  public int getRecordLength() {
    return 16;
  }
}
