import java.io.IOException;
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
public class Rename {
  private static final String regex = "([^\\s]+)\\s+(.+)";

  private static final String case_create = "Create";

  private static final String case_open = "Open";

  private static final String case_rename = "Rename";

  private static final String case_delete = "Delete";

  private static final String case_size = "Size";

  private static final String delimiter = "~";

  public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    public void map(LongWritable key, Text value, OutputCollector<Text, Text> output,
            Reporter reporter) throws IOException {
      String line = value.toString();

      Pattern p = Pattern.compile(regex);
      Matcher m = p.matcher(line);
      if (m.matches()) {
        String keySplitted[] = m.group(1).split(delimiter);
        if (keySplitted[0].equals(case_open)) {
          Text key_item = new Text(keySplitted[1] + delimiter + "1");
          Text val_item = new Text(case_open + delimiter + m.group(2));
          output.collect(key_item, val_item);
        } else if (keySplitted[0].equals(case_create)) {
          Text key_item = new Text(keySplitted[1] + delimiter + "1");
          Text val_item = new Text(case_create + delimiter + m.group(2));
          output.collect(key_item, val_item);
        } else if (keySplitted[0].equals(case_rename)) {
          String valSplitted[] = m.group(2).split(delimiter);
          Text key_item = new Text(valSplitted[0] + delimiter + "0");
          Text val_item = new Text(case_rename + delimiter + valSplitted[1]);
          output.collect(key_item, val_item);
        } else if (keySplitted[0].equals(case_delete)) {
          Text key_item = new Text(keySplitted[1] + delimiter + "1");
          Text val_item = new Text(case_delete + delimiter + m.group(2));
          output.collect(key_item, val_item);
        } else {
          Text key_item = new Text(m.group(1) + delimiter + "1");
          Text val_item = new Text(case_size + delimiter + m.group(2));
          output.collect(key_item, val_item);
        }
      }
    }
  }

  public static class FirstPartitioner implements Partitioner<Text, Text> {
    @Override
    public void configure(JobConf job) {
    }

    @Override
    public int getPartition(Text key, Text value, int numPartitions) {
      String file = key.toString().split(delimiter)[0];
      return Math.abs(file.hashCode()) % numPartitions;
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
        result = Integer.valueOf(keysSplitted1[1]).compareTo(Integer.valueOf(keysSplitted2[1]));
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

  public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output,
            Reporter reporter) throws IOException {
      String newPath = key.toString().split(delimiter)[0];
      while (values.hasNext()) {
        String currValue = values.next().toString();
        String valueSplitted[] = currValue.split(delimiter);
        if (valueSplitted[0].equals(case_open)) {
          Text key_item = new Text(valueSplitted[1] + delimiter + newPath);
          Text val_item = new Text(case_open + delimiter + valueSplitted[2]);
          output.collect(key_item, val_item);
        } else if (valueSplitted[0].equals(case_create)) {
          Text key_item = new Text(valueSplitted[1] + delimiter + newPath);
          Text val_item = new Text(case_create + delimiter + valueSplitted[2]);
          output.collect(key_item, val_item);
        } else if (valueSplitted[0].equals(case_delete)) {
          Text key_item = new Text(valueSplitted[1] + delimiter + newPath);
          Text val_item = new Text(case_delete + delimiter + valueSplitted[2]);
          output.collect(key_item, val_item);
        } else if (valueSplitted[0].equals(case_rename)) {
          newPath = valueSplitted[1];
        } else if (valueSplitted[0].equals(case_size)) {
          Text key_item = new Text(valueSplitted[1] + delimiter + newPath);
          Text val_item = new Text(case_size + delimiter + valueSplitted[2] + delimiter
                  + valueSplitted[3]);
          output.collect(key_item, val_item);
        }
      }
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(Rename.class);
    conf.setJobName("Rename");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

    conf.setMapperClass(Map.class);
    conf.setPartitionerClass(FirstPartitioner.class);
    conf.setOutputKeyComparatorClass(KeyComparator.class);
    conf.setOutputValueGroupingComparator(GroupComparator.class);
    conf.setReducerClass(Reduce.class);

    conf.setNumReduceTasks(40);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
  }
}