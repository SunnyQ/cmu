import java.io.IOException;
import java.util.ArrayList;
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
public class Ugi_Size {
  private static final String regex = "([^\\s]+)\\s+(.+)";

  private static final String regex_size = "(.+)\\s+(\\d+~\\d+)";

  private static final String case_create = "Create";

  private static final String case_open = "Open";

  private static final String case_rename = "Rename";

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
          Text key_item = new Text(m.group(2));
          Text val_item = new Text(case_open + delimiter + keySplitted[1]);
          output.collect(key_item, val_item);
        } else if (keySplitted[0].equals(case_create)) {
          Text key_item = new Text(m.group(2));
          Text val_item = new Text(case_create + delimiter + keySplitted[1]);
          output.collect(key_item, val_item);
        } else if (keySplitted[0].equals(case_rename)) {
          String valSplitted[] = m.group(2).split(delimiter);
          Text key_item = new Text(valSplitted[0]);
          Text val_item = new Text(case_rename + delimiter + keySplitted[1] + delimiter
                  + valSplitted[1]);
          output.collect(key_item, val_item);
        } else {
          Pattern p_size = Pattern.compile(regex_size);
          Matcher m_size = p_size.matcher(line);
          if (m_size.matches()) {
            Text key_item = new Text(m_size.group(1).trim());
            Text val_item = new Text(case_size + delimiter + m_size.group(2));
            output.collect(key_item, val_item);
          }
        }
      }
    }
  }

  public static class Reduce extends MapReduceBase implements
          Reducer<Text, Text, Text, LongWritable> {
    private long wSize;

    private long rSize;

    private String wUgi;

    private String newPath;

    private ArrayList<String> rUgis = new ArrayList<String>();

    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, LongWritable> output,
            Reporter reporter) throws IOException {
      wSize = 0;
      rSize = 0;
      while (values.hasNext()) {
        String currValue = values.next().toString();
        String valueSplitted[] = currValue.split(delimiter);
        if (valueSplitted[0].equals(case_open) && !rUgis.contains(valueSplitted[1])) {
          rUgis.add(valueSplitted[1]);
        } else if (valueSplitted[0].equals(case_create)) {
          wUgi = valueSplitted[1];
        } else if (valueSplitted[0].equals(case_rename)) {
          newPath = valueSplitted[2];
        } else if (valueSplitted[0].equals(case_size)) {
          wSize += Long.parseLong(valueSplitted[1]);
          rSize += Long.parseLong(valueSplitted[2]);
        }
      }

      newPath = (newPath == null) ? key.toString() : newPath;
      if (rUgis.size() > 0) {
        rSize /= rUgis.size();
        for (String rUgi : rUgis) {
          if (rUgi.equals(wUgi) && (wSize + rSize) > 0) {
            output.collect(new Text(rUgi + delimiter + newPath), new LongWritable(wSize + rSize));
          } else if (rSize > 0) {
            output.collect(new Text(rUgi + delimiter + newPath), new LongWritable(rSize));
          }
        }
      }

      if (wUgi == null && wSize > 0)
        output.collect(new Text("unknown_user" + delimiter + newPath), new LongWritable(wSize));
      else if (!rUgis.contains(wUgi) && wSize > 0)
        output.collect(new Text(wUgi + delimiter + newPath), new LongWritable(wSize));
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(Ugi_Size.class);
    conf.setJobName("Ugi_Size");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

    conf.setMapperClass(Map.class);
    conf.setReducerClass(Reduce.class);

    conf.setNumMapTasks(50);
    conf.setNumReduceTasks(40);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]));
    FileOutputFormat.setOutputPath(conf, new Path(args[1]));

    JobClient.runJob(conf);
  }
}