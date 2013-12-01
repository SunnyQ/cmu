package Client;

import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import util.JarExtractor;
import Communication.MapperTaskPayload;
import Communication.Message;
import Communication.ReducerTaskPayload;
import Communication.ServiceRunnable;
import Communication.StringMessagePayload;
import Configuration.ConfigParser;
import MapReduce.Implementation.HashPartitioner;
import MapReduce.Implementation.StringInputRecord;
import MapReduce.Implementation.StringWritable;
import MapReduce.Interface.Mapper;
import MapReduce.Interface.Reducer;
import MapReduce.JobControl.MapTask;
import MapReduce.JobControl.ReduceTask;

/**
 * Class: RequestHandlingService.java
 * 
 * The service handle all the requests from the server
 * 
 * @author Yang Sun
 * 
 */
public class RequestHandlingService implements ServiceRunnable {

  private boolean isAlive;

  private ServerSocket serviceSock;

  private String clientID;

  private ConfigParser parser;

  public RequestHandlingService(String clientID, ConfigParser parser) {
    this.parser = parser;
    this.clientID = clientID;
    isAlive = false;

    /* Initialise the server socket to receive the request from the master controller */
    try {
      serviceSock = new ServerSocket(parser.getClientPort(clientID));
    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  @Override
  public boolean isAlive() {
    return isAlive;
  }

  @SuppressWarnings("unchecked")
  @Override
  public void run() {
    Socket clientSock = null;
    Message incomingMsg = null;
    try {
      for (;;) {
        clientSock = serviceSock.accept();
        ObjectInputStream in = new ObjectInputStream(clientSock.getInputStream());
        incomingMsg = (Message) in.readObject();
        switch (incomingMsg.getType()) {

          case STATUS:
            /* Process the status checking request, simply relay if healthy */
            StringMessagePayload msgPayload = (StringMessagePayload) incomingMsg.getPayload();
            if (msgPayload.getMsg().equals("QUERY"))
              msgPayload.setMsg("HEALTHY");
            break;

          case MAPPER:
            /*
             * Process the map task request, start a new map task and relay back a running signal
             */
            MapperTaskPayload mapperPayload = (MapperTaskPayload) incomingMsg.getPayload();

            /* extract the mapper from the Jar file */
            JarExtractor mapExtractor = new JarExtractor(mapperPayload.getJarFile());
            Class<? extends Mapper<StringInputRecord, StringWritable, StringWritable>> mapCl = (Class<? extends Mapper<StringInputRecord, StringWritable, StringWritable>>) mapExtractor
                    .extractClass(mapperPayload.getMapperClass());
            Mapper<StringInputRecord, StringWritable, StringWritable> mapper = (Mapper<StringInputRecord, StringWritable, StringWritable>) mapCl
                    .newInstance();

            /* Prepare the parameter to run map task */
            long filesize = new File(mapperPayload.getInputFile()).length();
            long offset = mapperPayload.getMapperID() * (filesize / mapper.getNumMapper());
            long numRecordsRead = filesize / mapper.getRecordLength() / mapper.getNumMapper();
            String tmpMapDir = "tmp" + File.separator + clientID + File.separator
                    + mapperPayload.getTaskID() + File.separator + "map";
            new File(tmpMapDir).mkdirs();

            /* Create the map task and run */
            MapTask<StringInputRecord, StringWritable, StringWritable> mapTask = new MapTask<StringInputRecord, StringWritable, StringWritable>();
            mapTask.setSignalAddr(parser.getMasterAddr());
            mapTask.setSignalPort(parser.getMasterPort());
            mapTask.setResponseMsg(incomingMsg);
            mapTask.setFileOffset(offset);
            mapTask.setNumRecordsRead(numRecordsRead);
            mapTask.setInputRecordClass(StringInputRecord.class);
            mapTask.setInputFile(mapperPayload.getInputFile());
            mapTask.setMapperClass(mapCl);
            mapTask.setPartitionerClass(HashPartitioner.class);
            mapTask.setTmpDir(tmpMapDir);
            new Thread(mapTask).start();
            break;

          case REDUCER:
            /*
             * Process the reduce task request, start a new reduce task and relay back a running
             * signal
             */
            ReducerTaskPayload reducerPayload = (ReducerTaskPayload) incomingMsg.getPayload();

            /* extract the reducer from the Jar file */
            JarExtractor reduceExtractor = new JarExtractor(reducerPayload.getJarFile());
            Class<? extends Reducer<StringWritable, StringWritable>> reduceCl = (Class<? extends Reducer<StringWritable, StringWritable>>) reduceExtractor
                    .extractClass(reducerPayload.getReducerClass());

            /* Prepare the parameter to run reduce task */
            String tmpReduceDir = "tmp" + File.separator + clientID + File.separator
                    + reducerPayload.getTaskID() + File.separator + "reduce";
            new File(tmpReduceDir).mkdirs();

            /* Create the reduce task and run */
            ReduceTask<StringWritable, StringWritable> reduceTask = new ReduceTask<StringWritable, StringWritable>();
            reduceTask.setOutputIdentifier("output_" + reducerPayload.getTaskID() + "_" + clientID);
            reduceTask.setSignalAddr(parser.getMasterAddr());
            reduceTask.setSignalPort(parser.getMasterPort());
            reduceTask.setResponseMsg(incomingMsg);
            reduceTask.setOutputFolder(reducerPayload.getOutputFolder());
            reduceTask.setSplitPaths(reducerPayload.getInputSplits());
            reduceTask.setTmpDir(tmpReduceDir);
            reduceTask.setBufferSize(1000);
            reduceTask.setReducerClass(reduceCl);
            reduceTask.setOutputOffset(0);
            new Thread(reduceTask).start();
            break;

          case RESULT:
            /* Process the task running result from the master controller, simply print it out */
            StringMessagePayload resultPayload = (StringMessagePayload) incomingMsg.getPayload();
            System.out.println(resultPayload.getMsg());
            break;

          default:
            break;
        }

        /* as mentioned above, relay message back to the master */
        ObjectOutputStream out = new ObjectOutputStream(clientSock.getOutputStream());
        out.writeObject(incomingMsg);
        out.flush();
        out.close();
        clientSock.close();
      }
    } catch (IOException e) {
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } catch (InstantiationException e) {
      e.printStackTrace();
    } catch (IllegalAccessException e) {
      e.printStackTrace();
    }
  }

  @Override
  public void shutdown() throws IOException {
    if (!serviceSock.isClosed())
      serviceSock.close();
    isAlive = false;
  }

}
