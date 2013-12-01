package edu.cmu.andrew;

import java.util.HashSet;

public class LongestConsecutiveSequence {
    public int longestConsecutive(int[] num) {
        // Start typing your Java solution below
        // DO NOT write main() function
        HashSet<Integer> pool = new HashSet<Integer>();
        for (int i = 0; i < num.length; i ++) {
            pool.add(num[i]);
        }
        
        int maxLen = 0;
        for (int i = 0; i < num.length; i ++) {
            maxLen = Math.max(maxLen, 1 + getCount(pool, num[i] + 1, true) + getCount(pool, num[i] - 1, false));
        }
        
        return maxLen;
    }
    
    private int getCount(HashSet<Integer> pool, int target, boolean increasing) {
        if (!pool.contains(target)) {
            return 0;
        }
        
        pool.remove(target);
        return 1 + getCount(pool, increasing ? target + 1 : target - 1, increasing);
        
//        int count = 0;
//        while (pool.contains(target)) {
//            pool.remove(target);
//            count ++;
//            target = increasing ? target + 1 : target - 1;
//        }
//        return count;
    }

    public static void main(String args[]) {
        int[] num = {9,-8,9,8,-7,9,-4,6,5,5,6,7,-9,-5,-4,6,-8,-1,8,0,1,5,4};
        System.out.println(new LongestConsecutiveSequence().longestConsecutive(num));
    }
}
