package edu.cmu.andrew;

import java.util.ArrayList;
import java.util.Arrays;

public class ThreeSum{

    public static ArrayList<ArrayList<Integer>> threeSum(int[] num) {
        ArrayList<ArrayList<Integer>> res = new ArrayList<ArrayList<Integer>>();

        Arrays.sort(num);

        for (int i = 0; i < num.length; i++) {
            int j = i + 1;
            int k = num.length - 1;
            while (j < k) {
                int sum = num[j] + num[k] + num[i];
                if (sum == 0) {
                    System.out.println(i);
                    ArrayList<Integer> inner = new ArrayList<Integer>();
                    inner.add(num[i]);
                    inner.add(num[j]);
                    inner.add(num[k]);
                    res.add(inner);
                    break;
                } else if (sum > 0) {
                    k --;
                } else if (sum < 0) {
                    j ++;
                }
            }
        }
        return res;
    }
    
    
    public static void main(String[] args) {
        int[] num = {-1, 1, 0};
        System.out.println(threeSum(num));
    }
}
