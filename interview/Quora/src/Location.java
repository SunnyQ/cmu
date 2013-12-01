
/**
 * Quora - Nearby
 * Location.java
 * Purpose: represent the location of the topic
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class Location {
    private double x;
    private double y;

    public Location(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public double getX() {
        return x;
    }

    public double getY() {
        return y;
    }

    public Double getDistance(Location center) {
        return Math.pow(getX() - center.getX(), 2) + Math.pow(getY() - center.getY(), 2);
    }
}
