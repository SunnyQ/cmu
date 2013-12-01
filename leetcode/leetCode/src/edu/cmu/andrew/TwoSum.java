package edu.cmu.andrew;

import java.util.HashMap;
import java.util.Map;

/**
 * Given an array of integers, find two numbers such that they add up to a specific target number.
 * 
 * The function twoSum should return indices of the two numbers such that they add up to the target,
 * where index1 must be less than index2. Please note that your returned answers (both index1 and
 * index2) are not zero-based.
 * 
 * You may assume that each input would have exactly one solution.
 * 
 * Input: numbers={2, 7, 11, 15}, target=9 Output: index1=1, index2=2
 */

public class TwoSum {

  public static int[] twoSum(int[] numbers, int target) {
    // Start typing your Java solution below
    // DO NOT write main() function
    Map<Integer, Integer> pool = new HashMap<Integer, Integer>();
    int[] res = new int[2];

    for (int i = 0; i < numbers.length; i++) {
      if (pool.containsKey(target - numbers[i])) {
        res[0] = pool.get(target - numbers[i]) + 1;
        res[1] = i + 1;
      } else {
        pool.put(numbers[i], i);
      }
    }
    return res;
  }

  public static void main(String[] args) {
    int[] numbers = { 5, 75, 25 };
    int[] res = twoSum(numbers, 100);
    System.out.println(res[0] + ", " + res[1]);
  }
}
