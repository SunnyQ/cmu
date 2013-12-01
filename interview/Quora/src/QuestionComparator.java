import java.util.Comparator;

/**
 * Quora - Nearby
 * QuestionComparator.java
 * Purpose: Compare the question according the closest location to the center
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class QuestionComparator implements Comparator<Question> {

    private Location center;
    public QuestionComparator(Location center) {
        this.center = center;
    }

    
    @Override
    public int compare(Question o1, Question o2) {
        int res = o1.getClosestDistance(center).compareTo(o2.getClosestDistance(center));
        if (res != 0) {
            return res;
        }
        // return the higher id question if there is a tie
        return new Integer(o2.getId()).compareTo(o1.getId());
    }
}
