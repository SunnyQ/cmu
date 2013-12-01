import java.util.ArrayList;
import java.util.List;

/**
 * Quora - Nearby Question.java Purpose: represent a Question
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class Question {
    private int id;
    private List<Topic> associatedTopics;

    public Question(int id) {
        this.id = id;
        associatedTopics = new ArrayList<Topic>();
    }

    public void addAssociatedTopic(Topic t) {
        associatedTopics.add(t);
    }

    public int getId() {
        return id;
    }

    public int getAssociatedTopicsSize() {
        return associatedTopics.size();
    }

    /**
     * Given a location center, find the closest distance between the center and the topic location the question is
     * currently associated with
     * 
     * @param center
     * @return
     */
    public Double getClosestDistance(Location center) {
        if (associatedTopics.size() == 0) {
            return Double.MAX_VALUE;
        }
        Topic minDistanceTopic = associatedTopics.get(0);
        for (int i = 0; i < associatedTopics.size(); i++) {
            if (associatedTopics.get(i).getLocation().getDistance(center) < minDistanceTopic.getLocation().getDistance(
                    center)) {
                minDistanceTopic = associatedTopics.get(i);
            }
        }
        return minDistanceTopic.getLocation().getDistance(center);
    }

}
