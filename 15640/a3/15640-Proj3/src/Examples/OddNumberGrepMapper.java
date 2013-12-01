package Examples;

import java.io.IOException;

import MapReduce.Implementation.StringInputRecord;
import MapReduce.Implementation.StringWritable;
import MapReduce.Interface.Mapper;
import MapReduce.Records.IntermediateRecordCollector;

/**
 * Class: OddNumberGrepMapper.java
 * 
 * A mapper example to output only odd numbers
 * 
 * @author Yang Sun
 * 
 */
public class OddNumberGrepMapper implements
        Mapper<StringInputRecord, StringWritable, StringWritable> {

  @Override
  public void Map(StringInputRecord record,
          IntermediateRecordCollector<StringWritable, StringWritable> collector) {
    try {
      if (Integer.parseInt(record.toString()) % 2 == 1)
        collector.collect(new StringWritable("Odd"), new StringWritable("1"));
    } catch (IOException e) {
    }
  }

  @Override
  public int getRecordLength() {
    return 16;
  }

  @Override
  public int getNumMapper() {
    return 2;
  }

  @Override
  public int getNumReducer() {
    return 1;
  }

}
