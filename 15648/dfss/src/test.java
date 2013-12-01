import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class test {
  public static void main(String[] args) {
    String v = "abalacha~1  17375043";
    String regex_fileblk = "(.+)~(\\d+)\\s+(\\d+)$";
    Pattern p_fileblk = Pattern.compile(regex_fileblk);
    Matcher m_fileblk = p_fileblk.matcher(v);
    if (m_fileblk.matches()) {
      System.out.println(m_fileblk.group(1) + ", " + m_fileblk.group(3) + ".");
//      System.out.println("/data/clueweb/Category_B_inlinks/harvest/en0002/docOrder-52.warc.gz".hashCode());
    }
  }
}
