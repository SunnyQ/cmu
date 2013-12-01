package edu.cmu.andrew;

public class CountingSort {

    public CountingSort() {
        // TODO Auto-generated constructor stub
    }
    
    public int[] sort(int[] input, int k) {
        int[] count = new int[k];
        int[] res = new int[input.length];
        for (int i = 0; i < input.length; i ++) {
            count[input[i]] ++;
        }
        
        for (int i = 1; i < k; i ++) {
            count[i] += count[i - 1];
        }
        
        for (int i = 0; i < input.length; i ++) {
            res[count[input[i]] - 1] = input[i];
            count[input[i]] --;
        }
        
        return res;
    }
    
    public static void main(String[] args) {
        CountingSort cs = new CountingSort();
        int[] res = cs.sort(new int[]{1, 4, 2, 6, 3, 3, 9}, 10);
        for (int i = 0; i < res.length; i ++) {
            System.out.print(res[i] + " ");
        }
    }
}
