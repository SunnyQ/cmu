import java.io.IOException;
import java.util.Iterator;

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
public class Cal_NumFiles {
  private static final String delimiter = "~";

  public static class Map extends MapReduceBase implements
          Mapper<LongWritable, Text, Text, LongWritable> {

    public void map(LongWritable key, Text value, OutputCollector<Text, LongWritable> output,
            Reporter reporter) throws IOException {
      String line = value.toString();
      String[] keyValueSplitted = line.split("\\s+");
      String ugi = keyValueSplitted[0];
      long bytesize = Long.parseLong(keyValueSplitted[1]);

      output.collect(new Text(ugi), new LongWritable(bytesize));
      output.collect(new Text("all"), new LongWritable(bytesize));
    }
  }

  public static class Reduce extends MapReduceBase implements
          Reducer<Text, LongWritable, Text, LongWritable> {
    public void reduce(Text key, Iterator<LongWritable> values,
            OutputCollector<Text, LongWritable> output, Reporter reporter) throws IOException {
      long numFiles = 0;
      long totalbytes = 0;
      while (values.hasNext()) {
        numFiles++;
        long bytesize = values.next().get();
        output.collect(new Text(key.toString()), new LongWritable(bytesize));
        totalbytes += bytesize;
      }
      output.collect(new Text("Totalbytes" + delimiter + key.toString() + delimiter + numFiles),
              new LongWritable(totalbytes));
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(Cal_NumFiles.class);
    conf.setJobName("Cal_NumFiles");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(LongWritable.class);

    conf.setMapperClass(Map.class);
    conf.setReducerClass(Reduce.class);

    conf.setNumMapTasks(20);
    conf.setNumReduceTasks(100);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
  }
}