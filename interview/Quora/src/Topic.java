/**
 * Quora - Nearby
 * Topic.java
 * Purpose: represent a topic
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class Topic {
    private int id;
    private Location location;
    
    public Topic(int id, double x, double y) {
        this.id = id;
        this.location = new Location(x, y);
    }
    
    public int getId() {
        return id;
    }
    
    public Location getLocation() {
        return location;
    }
}
