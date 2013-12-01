import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * If the Fibonacci series is 1,2,3,5,8,13,.. then 10 can be written as 8 + 2
 * ==> 10010 and 17 can be written as 13 + 3 + 1 ==> 100101. Got it?? The
 * Question was, given n, I need to get all possible representations of n in
 * Fibonacci Binary Number System. as 10 = 8 + 2 ==> 10010 also 10 = 5 + 3 + 2
 * ==> 1110
 * 
 * @author Kobe
 * 
 */
public class FibToTarget {
    private ArrayList<Integer> findSumToK(int sum) {
        ArrayList<Integer> res = new ArrayList<Integer>();
        ArrayList<Integer> fibs = new ArrayList<Integer>();
        fibs.add(1);
        fibs.add(2);
        while (fibs.get(fibs.size() - 1) <= sum) {
            fibs.add(fibs.get(fibs.size() - 1) + fibs.get(fibs.size() - 2));
        }
        HashMap<Integer, Integer> cache = new HashMap<Integer, Integer>();
        for (int i = 0; i < fibs.size(); i++) {
            cache.put(fibs.get(i), i);
        }
        findSumToKHelper(cache, fibs, res, sum, 0);
        return res;
    }

    private void findSumToKHelper(HashMap<Integer, Integer> cache,
            List<Integer> fibs, ArrayList<Integer> res, int sum, Integer partial) {
        if (sum == 0) {
            res.add(partial);
        }

        if (sum <= 0) {
            return;
        }

        for (int i = 0; i < fibs.size(); i++) {
            findSumToKHelper(cache, fibs.subList(i + 1, fibs.size()), res, sum
                    - fibs.get(i), (int) Math.pow(10, cache.get(fibs.get(i)))
                    + partial);
        }
    }

    public static void main(String[] args) {
        FibToTarget ftt = new FibToTarget();
        System.out.println(ftt.findSumToK(17));
    }
}
