package cmu.edu.ds.a1.MP;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.URL;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import cmu.edu.ds.a1.IF.MigratableProcess;
import cmu.edu.ds.a1.IO.TransactionalFileInputStream;
import cmu.edu.ds.a1.IO.TransactionalFileOutputStream;

/**
 * The Class WebCrawler is a basic web crawler. It takes one file of seed URLs, where each line is a
 * seed URL. Then it continuously crawls all the pages until a maximum number of pages have been
 * crawled.
 */

public class WebCrawler implements MigratableProcess {

  /** The Constant serialVersionUID. */
  private static final long serialVersionUID = -8202960348049692843L;

  /** The suspending flag. */
  private volatile boolean suspending;

  /** The input stream. */
  private TransactionalFileInputStream inStream;

  /** The output stream. */
  private TransactionalFileOutputStream outStream;

  /** The URL queue. */
  private Queue<String> URLQueue = new LinkedList<String>();

  /** The crawled URL set. */
  private Set<String> crawledURLSet = new HashSet<String>();

  /** The max number of pages to be crawled. */
  private int maxNumber = -1;

  /** The regexp and pattern used to parse URLs. */
  String regexp = "http://(\\w+\\.)*(\\w+)";

  Pattern pattern = Pattern.compile(regexp);

  /** The arguments. */
  private String[] args;

  /**
   * Instantiates a new web crawler.
   * 
   * @param args
   *          the arguments
   * @throws Exception
   */
  public WebCrawler(String[] args) throws Exception {
    if (args.length < 3) {
      System.out.println("usage: WebCrawler <seedFile> <outputFile> <maxNumberOfCrawledPage>");
      throw new Exception("Invalid Arguments");

    }
    this.maxNumber = Integer.parseInt(args[2]);
    if (!new File(args[0]).isFile()) {
      System.out.println("seedFile \"" + args[0] + "\" is not a valid input file!");
      throw new Exception("Invalid Arguments");
    }
    this.inStream = new TransactionalFileInputStream(args[0]);
    this.outStream = new TransactionalFileOutputStream(args[1], false);
    this.args = args;
  }

  /**
   * Crawl one page.
   */
  private void crawlOnePage() {

    String url = URLQueue.poll();
    if (crawledURLSet.contains(url))
      return;

    StringBuilder sb = new StringBuilder();
    boolean succeed = true;
    try {
      URL my_url = new URL(url);
      BufferedReader br = new BufferedReader(new InputStreamReader(my_url.openStream()));
      String strTemp = "";
      while (null != (strTemp = br.readLine())) {
        sb.append(strTemp);
      }
      br.close();
    } catch (Exception e) {
      succeed = false;
    }

    if (!succeed) {
      return;
    }

    // add this page to crawled pages
    crawledURLSet.add(url);

    // output the page
    String content = sb.toString();
    PrintStream out = new PrintStream(outStream);
    out.println(url);
    out.println(content);
    out.println();
    out.flush();

    // parse new URLs
    Matcher matcher = pattern.matcher(sb.toString());
    while (matcher.find()) {
      String newURL = matcher.group();
      if (!crawledURLSet.contains(newURL)) {
        URLQueue.add(newURL);
      }
    }
  }

  @SuppressWarnings("deprecation")
  private void loadSeedFile() {
    DataInputStream in = new DataInputStream(inStream);

    while (!suspending) {
      String url = null;
      try {
        url = in.readLine();
      } catch (IOException e) {
        break;
      }

      if (url == null)
        break;

//      System.out.println(url);
      URLQueue.add(url);
    }
  }

  private void crawl() {
    while (!URLQueue.isEmpty() && (crawledURLSet.size() < maxNumber) && !suspending)
      crawlOnePage();
  }

  /*
   * @see java.lang.Runnable#run()
   */
  @Override
  public void run() {
    loadSeedFile();
    crawl();
    suspending = false;
  }

  /*
   * @see cmu.edu.ds.a1.IF.MigratableProcess#suspend()
   */
  @Override
  public void suspend() {
    suspending = true;
    while (suspending)
      ;
    // System.out.println("suspending...");
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder("WebCrawler");
    for (String s : args)
      sb.append(" " + s);
    return sb.toString();
  }

  /**
   * The main method.
   * 
   * @param args
   *          the arguments
   * @throws Exception
   */
  public static void main(String[] args) throws Exception {

    String[] tmpArgs = { "seed.txt", "test.txt", "20" };
    WebCrawler crawler = new WebCrawler(tmpArgs);

    Thread t = new Thread(crawler);
    t.start();

    try {
      Thread.sleep(10);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }

    System.out.println("1");
    crawler.suspend();
    System.out.println("2");

    try {
      Thread.sleep(2000);
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    System.out.println("3");
    t = new Thread(crawler);
    System.out.println("4");
    t.start();
    System.out.println("5");
  }

}
