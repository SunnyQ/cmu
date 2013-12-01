import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Partitioner;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.TextInputFormat;
import org.apache.hadoop.mapred.TextOutputFormat;

@SuppressWarnings({ "rawtypes", "deprecation" })
public class Final_Result {
  private static final String regex_totalsize = "^Totalbytes~(.+)~([^\\s]+)\\s+(.+)$";

  private static final String regex_ugisize = "([^\\s]+)\\s+(\\d+)$";

  private static final String delimiter = "~";

  public static class Map extends MapReduceBase implements
          Mapper<LongWritable, Text, Text, LongWritable> {

    public void map(LongWritable key, Text value, OutputCollector<Text, LongWritable> output,
            Reporter reporter) throws IOException {
      String line = value.toString();
      Pattern p_totalsize = Pattern.compile(regex_totalsize);
      Matcher m_totalsize = p_totalsize.matcher(line);

      Pattern p_ugisize = Pattern.compile(regex_ugisize);
      Matcher m_ugisize = p_ugisize.matcher(line);

      if (m_totalsize.matches()) {
        Text key_item = new Text(m_totalsize.group(1) + delimiter + "0");
        LongWritable val_item = new LongWritable(Long.parseLong(m_totalsize.group(2)));
        output.collect(key_item, val_item);
      } else if (m_ugisize.matches()) {
        Text key_item = new Text(m_ugisize.group(1) + delimiter + m_ugisize.group(2));
        LongWritable val_item = new LongWritable(Long.parseLong(m_ugisize.group(2)));
        output.collect(key_item, val_item);
      }
    }
  }

  public static class FirstPartitioner implements Partitioner<Text, LongWritable> {
    @Override
    public void configure(JobConf job) {
    }

    @Override
    public int getPartition(Text key, LongWritable value, int numPartitions) {
      String ugi = key.toString().split(delimiter)[0];
      return Math.abs(ugi.hashCode()) % numPartitions;
    }
  }

  public static class KeyComparator extends WritableComparator {
    protected KeyComparator() {
      super(Text.class, true);
    }

    @Override
    public int compare(WritableComparable w1, WritableComparable w2) {
      String[] keysSplitted1 = ((Text) w1).toString().split(delimiter);
      String[] keysSplitted2 = ((Text) w2).toString().split(delimiter);

      int result = keysSplitted1[0].compareTo(keysSplitted2[0]);
      if (result == 0) {
        result = Long.valueOf(keysSplitted1[1]).compareTo(Long.valueOf(keysSplitted2[1]));
      }
      return result;
    }
  }

  public static class GroupComparator extends WritableComparator {
    protected GroupComparator() {
      super(Text.class, true);
    }

    @Override
    public int compare(WritableComparable w1, WritableComparable w2) {
      String[] keysSplitted1 = ((Text) w1).toString().split(delimiter);
      String[] keysSplitted2 = ((Text) w2).toString().split(delimiter);
      return keysSplitted1[0].compareTo(keysSplitted2[0]);
    }
  }

  public static class Reduce extends MapReduceBase implements
          Reducer<Text, LongWritable, Text, Text> {

    public void reduce(Text key, Iterator<LongWritable> values, OutputCollector<Text, Text> output,
            Reporter reporter) throws IOException {
      StringBuilder outputStr = new StringBuilder();
      int p = 0;
      int counter = 0;
      long preVal = 0;
      long totalbytes = 0;

      String[] keySplitted = key.toString().split(delimiter);
      String ugi = keySplitted[0];
      int numFiles = Integer.parseInt(values.next().toString());
      ArrayList<Integer> interested = getInterestedPoints(numFiles - 1);

      while (values.hasNext()) {
        long currValue = values.next().get();
        while (p < interested.size() && interested.get(p) == counter) {
          long curVal = Math.round(Math.log10((double) currValue));
          outputStr.append(curVal - preVal);
          outputStr.append(" ");
          preVal = curVal;
          p++;
        }
        counter++;
        totalbytes += currValue;
      }

      outputStr.append(Math.round(Math.log10((double) totalbytes)) - preVal);
      output.collect(new Text(ugi), new Text(outputStr.toString()));

    }

    private ArrayList<Integer> getInterestedPoints(int numFiles) {
      ArrayList<Integer> interested = new ArrayList<Integer>();
      interested.add((int) Math.round(0.0 * numFiles));
      interested.add((int) Math.round(0.00001 * numFiles));
      interested.add((int) Math.round(0.0001 * numFiles));
      interested.add((int) Math.round(0.001 * numFiles));
      interested.add((int) Math.round(0.01 * numFiles));
      interested.add((int) Math.round(0.1 * numFiles));
      interested.add((int) Math.round(0.25 * numFiles));
      interested.add((int) Math.round(0.5 * numFiles));
      interested.add((int) Math.round(0.75 * numFiles));
      interested.add((int) Math.round(0.9 * numFiles));
      interested.add((int) Math.round(0.99 * numFiles));
      interested.add((int) Math.round(0.999 * numFiles));
      interested.add((int) Math.round(0.9999 * numFiles));
      interested.add((int) Math.round(0.99999 * numFiles));
      interested.add((int) Math.round(1.0 * numFiles));
      return interested;
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(Final_Result.class);
    conf.setJobName("Final_Result");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(LongWritable.class);

    conf.setMapperClass(Map.class);
    conf.setPartitionerClass(FirstPartitioner.class);
    conf.setOutputKeyComparatorClass(KeyComparator.class);
    conf.setOutputValueGroupingComparator(GroupComparator.class);
    conf.setReducerClass(Reduce.class);

    conf.setNumMapTasks(100);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
  }
}