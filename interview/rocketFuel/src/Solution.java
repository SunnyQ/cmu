/* Enter your code here. Read input from STDIN. Print output to STDOUT */
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class Solution {
    public static class Racer {
        private int id;
        private long startTime;
        private long endTime;
        private int score;
        private int startPosition;
        
        /**
         * The racer object
         * @param id the racer id
         * @param startTime the racer start time
         * @param endTime the racer end time
         */
        public Racer(int id, long startTime, long endTime) {
            this.id = id;
            this.startTime = startTime;
            this.endTime = endTime;
            this.score = 0;
            this.startPosition = 0;
        }
        
        @Override
        public String toString() {
            return id + " " + score;
        }
    }
    
    private class StartTimeComparator implements Comparator<Racer> {
        @Override
        public int compare(Racer o1, Racer o2) {
            return new Long(o1.startTime).compareTo(new Long(o2.startTime));
        }
    }
    
    private class EndTimeComparator implements Comparator<Racer> {
        @Override
        public int compare(Racer o1, Racer o2) {
            return new Long(o1.endTime).compareTo(new Long(o2.endTime));
        }
    }
    
    private class ScoreIdComparator implements Comparator<Racer> {
        @Override
        public int compare(Racer o1, Racer o2) {
            int result = new Integer(o1.score).compareTo(new Integer(o2.score));
            if (result != 0) {
                return result;
            }
            return new Integer(o1.id).compareTo(new Integer(o2.id));
        }
    }
    
    private ArrayList<Racer> racers;
    public Solution(ArrayList<Racer> racers) {
        this.racers = racers;
    }
    
    public void rankRacers() {
        // sort the racers in ascending start time order
        Collections.sort(racers, new StartTimeComparator());
        for (int i = 0; i < racers.size(); i ++) {
            // record their start time order
            racers.get(i).startPosition = i;
        }
        
        // sort the racers in ascending end time order
        Collections.sort(racers, new EndTimeComparator());
        ArrayList<Integer> startPositions = new ArrayList<Integer>();
        // the racer we are looking for are those whose finish time is earlier and start time is later
        // the ascending end time order can make sure larger index racer has later finish time
        // in other words, all earlier processed racers have earlier finish time
        for (int i = 0; i < racers.size(); i ++) {
            // so the only thing left is to make sure the start time is later, use binary search - O(logn)
            int position = Collections.binarySearch(startPositions, racers.get(i).startPosition);
            position = (position < 0) ? -(position + 1) : position;
            // all the racers appear after ith racer in startPosition are those we are looking for (have later start time)
            racers.get(i).score = startPositions.size() - position;
            startPositions.add(position, racers.get(i).startPosition);
        }
        // finally, sort the result racer list basd on requirement
        Collections.sort(racers, new ScoreIdComparator());
    }
    
    public void printAllRacerResults() {
        for (int i = 0; i < racers.size(); i ++) {
            System.out.println(racers.get(i).toString());
        }
    }
    
    public ArrayList<Racer> getRacers() {
        return racers;
    }
   
    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String line = br.readLine();
        int n = Integer.parseInt(line);
        ArrayList<Racer> racers = new ArrayList<Racer>();
        while ((line = br.readLine()) != null) {
            String[] values = line.split(" ");
            racers.add(new Racer(Integer.parseInt(values[0]), Long.parseLong(values[1]), Long.parseLong(values[2])));
        }
        
        Solution solution = new Solution(racers);
        solution.rankRacers();
        solution.printAllRacerResults();
    }
}