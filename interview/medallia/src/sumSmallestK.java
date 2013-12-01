/* Enter your code here. Read input from STDIN. Print output to STDOUT */
import java.util.*;
import java.io.*;

public class sumSmallestK {

    /**
     * Prints out the sum of the k smallest {@link Integer}s from input. For example, given the
     * input ([-2, 4, 5, 2, 6, 7], 2), the output should be 0.
     *
     * @param input a list of {@link Integer}s of arbitrary ordering and range.
     * @param k the number of {@link Integer}s to print
     */
    public sumSmallestK(List<Integer> input, int k) {
//        Comparator<Integer> comparator = new Integer
        PriorityQueue<Integer> queue = new PriorityQueue<Integer>(k);
        for (int i = 0; i < input.size(); i ++) {
            queue.add(input.get(i));
        }
        
        int sum = 0;
        for (int i = 0; i < k; i ++) {
            sum += queue.poll();
        }
        System.out.println(sum);
    }

    public static void main(String[] args) throws Exception{
        List<Integer> input = new ArrayList<Integer>();
        BufferedReader br = null;
        try {
            br = new BufferedReader(new InputStreamReader(System.in));
            int k = Integer.parseInt(br.readLine());
            String line = null;
            while ((line = br.readLine()) != null && !line.isEmpty()) {
                input.add(Integer.parseInt(line));
            }
            new sumSmallestK(input, k);
        } finally {
            if (br != null)
                br.close();
        }
    }
}