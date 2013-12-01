import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.accumulo.core.client.mapreduce.AccumuloOutputFormat;
import org.apache.accumulo.core.data.Mutation;
import org.apache.accumulo.core.data.Value;
import org.apache.accumulo.core.util.CachedConfiguration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.TextInputFormat;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

public class DumpData2Tab extends Configured implements Tool {
  private static final String finalTable = "yksun_finaltab";

  private static final String username = "yksun";

  private static final String password = "19881015";

  private static final String zookeeperInstance = "accumulo";

  private static final String zookeeperServer = "zk2:2181,zk3:2181,zk4:2181";

  private static final String regex = "([^\\s]+)\\s+(.+)";

  private static final String case_create = "Create";

  private static final String case_open = "Open";

  private static final String case_delete = "Delete";

  private static final String case_size = "Size";

  private static final String delimiter = "~";

  public static class Map extends Mapper<LongWritable, Text, Text, Text> {

    @Override
    public void map(LongWritable key, Text value, Context output) throws IOException {
      try {
        String line = value.toString();
        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(line);
        if (m.matches()) {
          output.write(new Text(m.group(1)), new Text(m.group(2)));
        }
      } catch (InterruptedException e) {
        throw new IOException();
      }
    }
  }

  public static class Reduce extends Reducer<Text, Text, Text, Mutation> {
    protected void reduce(Text key, Iterable<Text> values, Context output) throws IOException,
            InterruptedException {
      ArrayList<String> readers = new ArrayList<String>();
      Iterator<Text> iter = values.iterator();
      String creator = "unknown_user";
      boolean isDeleted = false;
      String deletor = "unknown_user";
      Long rSize = 0L;
      Long wSize = 0L;

      while (iter.hasNext()) {
        String currValue = iter.next().toString();
        String valueSplitted[] = currValue.split(delimiter);
        if (valueSplitted[0].equals(case_size)) {
          rSize += Long.parseLong(valueSplitted[2]);
          wSize += Long.parseLong(valueSplitted[1]);
        } else if (valueSplitted[0].equals(case_open)) {
          if (!readers.contains(valueSplitted[1]))
            readers.add(valueSplitted[1]);
        } else if (valueSplitted[0].equals(case_create)) {
          creator = valueSplitted[1];
        } else if (valueSplitted[0].equals(case_delete)) {
          isDeleted = true;
          deletor = valueSplitted[1];
        }
      }

      if (wSize > 0) {
        Mutation mutation = new Mutation(new Text(creator + delimiter + key.toString()));
        mutation.put(new Text("size"), new Text("write"), new Value(wSize.toString().getBytes()));
        output.write(new Text(finalTable), mutation);
      }

      if (rSize > 0 && readers.size() == 0) {
        Mutation mutation = new Mutation(new Text("unknown_user" + delimiter + key.toString()));
        mutation.put(new Text("size"), new Text("read"), new Value(rSize.toString().getBytes()));
        output.write(new Text(finalTable), mutation);
      } else if (rSize > 0 && readers.size() > 0) {
        rSize /= readers.size();
        for (String ugi : readers) {
          Mutation mutation = new Mutation(new Text(ugi + delimiter + key.toString()));
          mutation.put(new Text("size"), new Text("read"), new Value(rSize.toString().getBytes()));
          output.write(new Text(finalTable), mutation);
        }
      }

      if (isDeleted) {
        Mutation mutation = new Mutation(new Text(deletor + delimiter + key.toString()));
        mutation.put(new Text("deletor"), new Text("boolean"), new Value("true".getBytes()));
        output.write(new Text(finalTable), mutation);
      }
    }
  }

  @Override
  public int run(String[] args) throws Exception {
    Job job = new Job(getConf(), DumpData2Tab.class.getName());
    job.setJarByClass(this.getClass());

    job.setInputFormatClass(TextInputFormat.class);
    TextInputFormat.setInputPaths(job, new Path(args[0]));

    job.setMapperClass(Map.class);
    job.setReducerClass(Reduce.class);

    job.setNumReduceTasks(40);

    job.setOutputFormatClass(AccumuloOutputFormat.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Mutation.class);
    job.setMapOutputValueClass(Text.class);

    AccumuloOutputFormat.setOutputInfo(job.getConfiguration(), username, password.getBytes(), true,
            finalTable);
    AccumuloOutputFormat.setZooKeeperInstance(job.getConfiguration(), zookeeperInstance,
            zookeeperServer);
    job.waitForCompletion(true);
    return 0;
  }

  public static void main(String[] args) throws Exception {
    int res = ToolRunner.run(CachedConfiguration.getInstance(), new DumpData2Tab(), args);
    System.exit(res);
  }
}