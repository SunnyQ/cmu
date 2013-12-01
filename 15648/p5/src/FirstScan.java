import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
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
public class FirstScan {
  private static final String regex_findicator = "^.*BLOCK\\* NameSystem.allocateBlock: (.+). (.+)";

  private static final String regex_windicator = "^([^\\s]+).*BLOCK\\* NameSystem.addStoredBlock: blockMap updated:.*is added to (.+) size (\\d+)$";

  private static final String regex_rindicator = "^([^\\s]+).*bytes: (\\d+), op: HDFS_READ.*blockid: (.+)";

  private static final String regex_wuindicator = "^([^\\s]+).*ugi=([^,]+),.*cmd=create\\s+src=([^\\s]+).*";

  private static final String regex_ruindicator = "^([^\\s]+).*ugi=([^,]+),.*cmd=open\\s+src=([^\\s]+).*";

  private static final String regex_duindicator = "^([^\\s]+).*ugi=([^,]+),.*cmd=delete\\s+src=([^\\s]+).*";

  private static final String regex_rename = ".*ugi=([^,]+),.*cmd=rename\\s+src=([^\\s]+).*dst=([^\\s]+).*";

  private static final String case_findicator = "F";

  private static final String case_windicator = "W";

  private static final String case_rindicator = "R";

  private static final String case_create = "Create";

  private static final String case_open = "Open";

  private static final String case_delete = "Delete";

  private static final String case_rename = "Rename";

  private static final String delimiter = "~";

  public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {

    public void map(LongWritable key, Text value, OutputCollector<Text, Text> output,
            Reporter reporter) throws IOException {
      String line = value.toString();

      if (line.contains("BLOCK* NameSystem")) {
        Pattern p_findicator = Pattern.compile(regex_findicator);
        Matcher m_findicator = p_findicator.matcher(line);
        if (m_findicator.matches()) {
          Text key_item = new Text(m_findicator.group(2));
          Text val_item = new Text(case_findicator + delimiter + m_findicator.group(1));
          output.collect(key_item, val_item);
          return;
        }

        Pattern p_windicator = Pattern.compile(regex_windicator);
        Matcher m_windicator = p_windicator.matcher(line);
        if (m_windicator.matches()) {
          Text key_item = new Text(m_windicator.group(2));
          Text val_item = new Text(case_windicator + delimiter + m_windicator.group(1) + delimiter
                  + m_windicator.group(3));
          output.collect(key_item, val_item);
          return;
        }
      } else if (line.contains("cmd=")) {
        Pattern p_wuindicator = Pattern.compile(regex_wuindicator);
        Matcher m_wuindicator = p_wuindicator.matcher(line);
        if (m_wuindicator.matches()) {
          Text key_item = new Text(case_create + delimiter + m_wuindicator.group(3));
          Text val_item = new Text(m_wuindicator.group(1) + delimiter + m_wuindicator.group(2));
          output.collect(key_item, val_item);
          return;
        }

        Pattern p_ruindicator = Pattern.compile(regex_ruindicator);
        Matcher m_ruindicator = p_ruindicator.matcher(line);
        if (m_ruindicator.matches()) {
          Text key_item = new Text(case_open + delimiter + m_ruindicator.group(3));
          Text val_item = new Text(m_ruindicator.group(1) + delimiter + m_ruindicator.group(2));
          output.collect(key_item, val_item);
          return;
        }

        Pattern p_duindicator = Pattern.compile(regex_duindicator);
        Matcher m_duindicator = p_duindicator.matcher(line);
        if (m_duindicator.matches()) {
          Text key_item = new Text(case_delete + delimiter + m_duindicator.group(3));
          Text val_item = new Text(m_duindicator.group(1) + delimiter + m_duindicator.group(2));
          output.collect(key_item, val_item);
          return;
        }

        Pattern p_rename = Pattern.compile(regex_rename);
        Matcher m_rename = p_rename.matcher(line);
        if (m_rename.matches()) {
          Text key_item = new Text(case_rename + delimiter + m_rename.group(1));
          Text val_item = new Text(m_rename.group(2) + delimiter + m_rename.group(3));
          output.collect(key_item, val_item);
          return;
        }
      } else {
        Pattern p_rindicator = Pattern.compile(regex_rindicator);
        Matcher m_rindicator = p_rindicator.matcher(line);
        if (m_rindicator.matches()) {
          Text key_item = new Text(m_rindicator.group(3));
          Text val_item = new Text(case_rindicator + delimiter + m_rindicator.group(1) + delimiter
                  + m_rindicator.group(2));
          output.collect(key_item, val_item);
          return;
        }
      }
    }
  }

  public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
    public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output,
            Reporter reporter) throws IOException {
      String filepath = "unknown_file";
      String keyStr = key.toString();
      HashMap<String, ArrayList<Long>> dates = new HashMap<String, ArrayList<Long>>();
      while (values.hasNext()) {
        if (keyStr.startsWith(case_create) || keyStr.startsWith(case_open)
                || keyStr.startsWith(case_rename) || keyStr.startsWith(case_delete)) {
          output.collect(key, values.next());
        } else {
          String currValue = values.next().toString();
          String valueSplitted[] = currValue.split(delimiter);
          if (valueSplitted[0].equals(case_findicator)) {
            filepath = valueSplitted[1];
          } else if (valueSplitted[0].equals(case_windicator)) {
            if (!dates.containsKey(valueSplitted[1])) {
              dates.put(valueSplitted[1], new ArrayList<Long>());
              dates.get(valueSplitted[1]).add(0L);
              dates.get(valueSplitted[1]).add(0L);
            }
            ArrayList<Long> curDate = dates.get(valueSplitted[1]);
            curDate.set(0, curDate.get(0) + Long.parseLong(valueSplitted[2]));
          } else if (valueSplitted[0].equals(case_rindicator)) {
            if (!dates.containsKey(valueSplitted[1])) {
              dates.put(valueSplitted[1], new ArrayList<Long>());
              dates.get(valueSplitted[1]).add(0L);
              dates.get(valueSplitted[1]).add(0L);
            }
            ArrayList<Long> curDate = dates.get(valueSplitted[1]);
            curDate.set(1, curDate.get(1) + Long.parseLong(valueSplitted[2]));
          }
        }
      }

      for (String date : dates.keySet()) {
        output.collect(new Text(filepath), new Text(date + delimiter + dates.get(date).get(0)
                + delimiter + dates.get(date).get(1)));
      }
    }
  }

  public static void main(String[] args) throws Exception {
    JobConf conf = new JobConf(FirstScan.class);
    conf.setJarByClass(FirstScan.class);
    conf.setJobName("FirstScan");

    conf.setOutputKeyClass(Text.class);
    conf.setOutputValueClass(Text.class);

    conf.setMapperClass(Map.class);
    conf.setReducerClass(Reduce.class);

    conf.setNumReduceTasks(30);

    conf.setInputFormat(TextInputFormat.class);
    conf.setOutputFormat(TextOutputFormat.class);

    FileInputFormat.setInputPaths(conf, new Path(args[0]), new Path(args[1]));
    FileOutputFormat.setOutputPath(conf, new Path(args[2]));

    JobClient.runJob(conf);
  }
}