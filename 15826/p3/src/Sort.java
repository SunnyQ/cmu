/**
    Note to 15-826 students
    You will most likely wish to use this example as the starting 
    point for all four Hadoop problems on you homework. This should
    be the only piece of code you use for the Hadoop questions that
    you do not write yourself.
 */
import java.io.IOException;
import java.util.Iterator;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.io.WritableComparator;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Partitioner;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

@SuppressWarnings("rawtypes")
public class Sort {

  private static final String delimiter = "__";

  public static class SortMapper extends Mapper<Object, Text, Text, Text> {

    private Text valText = new Text();

    private Text keyText = new Text();

    public void map(Object key, Text value, Context context) throws IOException,
            InterruptedException {
      String[] lineSplitted = value.toString().split("\t");
      valText.set(lineSplitted[0]);
      keyText.set(lineSplitted[1] + delimiter + lineSplitted[0]);
      context.write(keyText, valText);
    }
  }

  public static class FirstPartitioner extends Partitioner<Text, Text> {
    @Override
    public int getPartition(Text key, Text value, int numPartitions) {
      return Math.abs(key.toString().split(delimiter)[0].hashCode()) % numPartitions;
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

      int result = -1
              * Integer.valueOf(keysSplitted1[0]).compareTo(Integer.valueOf(keysSplitted2[0]));
      if (result == 0) {
        result = keysSplitted1[1].compareTo(keysSplitted2[1]);
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

  public static class SortReducer extends Reducer<Text, Text, Text, Text> {
    public void reduce(Text key, Iterable<Text> values, Context context) throws IOException,
            InterruptedException {
      Text val = new Text(key.toString().split(delimiter)[0]);
      Iterator<Text> iter = values.iterator();
      while (iter.hasNext()) {
        context.write(new Text(iter.next()), val);
      }
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    if (args.length != 2) {
      System.err.println("Usage: Sort <in> <out>");
      System.exit(2);
    }
    Job job = new Job(conf, "Sort");
    job.setJarByClass(Sort.class);
    job.setMapperClass(SortMapper.class);
    job.setPartitionerClass(FirstPartitioner.class);
    job.setSortComparatorClass(KeyComparator.class);
    job.setGroupingComparatorClass(GroupComparator.class);
    job.setReducerClass(SortReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
