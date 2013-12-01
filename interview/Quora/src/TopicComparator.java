import java.util.Comparator;

/**
 * Quora - Nearby
 * TopicComparator.java
 * Purpose: Compare the topic depending on the distance between the location and the center
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class TopicComparator implements Comparator<Topic> {

    private Location center;

    public TopicComparator(Location center) {
        this.center = center;
    }

    @Override
    public int compare(Topic o1, Topic o2) {
        int res = o1.getLocation().getDistance(center).compareTo(o2.getLocation().getDistance(center));
        if (res != 0) {
            return res;
        }
        // return the higher id topic if there is a tie
        return new Integer(o2.getId()).compareTo(o1.getId());
    }

}
