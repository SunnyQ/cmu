import java.io.IOException;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.FileInputFormat;
import org.apache.hadoop.mapred.FileOutputFormat;
import org.apache.hadoop.mapred.JobClient;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapred.MapReduceBase;
import org.apache.hadoop.mapred.Mapper;
import org.apache.hadoop.mapred.OutputCollector;
import org.apache.hadoop.mapred.Reducer;
import org.apache.hadoop.mapred.Reporter;
import org.apache.hadoop.mapred.TextInputFormat;
import org.apache.hadoop.mapred.TextOutputFormat;

@SuppressWarnings("deprecation")
public class Rename_Merge {
  private static final String regex = "(.+)\\s+(\\d+)";

  private static final String delimiter = "~";

  public static class Map extends MapReduceBase implements
          Mapper<LongWritable, Text, Text, LongWritable> {

    public void map(LongWritable key, Text value, OutputCollector<Text, LongWritable> output,
            Reporter reporter) throws IOException {
      String line = value.toString();

      Pattern p = Pattern.compile(regex);
      Matcher m = p.matcher(line);
      if (m.matches()) {
        Text key_item = new Text(m.group(1).trim());
        LongWritable val_item = new LongWritable(Long.parseLong(m.group(2)));
        output.collect(key_item, val_item);
      }
    }
  }

  public static class Reduce_1 extends MapReduceBase implements
          Reducer<Text, LongWritable, Text, LongWritable> {
    private long totalSize;

    public void reduce(Text key, Iterator<LongWritable> values,
            OutputCollector<Text, LongWritable> output, Reporter reporter) throws IOException {
      totalSize = 0;
      while (values.hasNext()) {
        totalSize += values.next().get();
      }
      output.collect(key, new LongWritable(totalSize));
    }
  }

  public static class Reduce_2 extends MapReduceBase implements
          Reducer<Text, LongWritable, Text, LongWritable> {
    private long totalSize;

    public void reduce(Text key, Iterator<LongWritable> values,
            OutputCollector<Text, LongWritable> output, Reporter reporter) throws IOException {
      totalSize = 0;
      while (values.hasNext()) {
        totalSize += values.next().get();
      }
      output.collect(new Text(key.toString().split(delimiter)[0]), new LongWritable(totalSize));
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(Rename_Merge.class);
    conf.setJobName("Rename_Merge");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(LongWritable.class);

    conf.setMapperClass(Map.class);
    conf.setCombinerClass(Reduce_1.class);
    conf.setReducerClass(Reduce_2.class);

    conf.setNumMapTasks(40);
    conf.setNumReduceTasks(20);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
  }
}