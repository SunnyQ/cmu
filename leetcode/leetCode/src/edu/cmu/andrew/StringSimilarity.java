package edu.cmu.andrew;

public class StringSimilarity {
    public int getCount(String input) {
        int total = input.length();
        for (int i = 1; i < input.length(); i ++) {
            int sum = 0;
            for (int j = 0; i + j < input.length(); j ++) {
                if (input.charAt(j) == input.charAt(i + j)) {
                    sum ++;
                } else {
                    break;
                }
            }
            total += sum;
        }
        return total;
    }
    
    public static void main(String[] args) {
        System.out.println(new StringSimilarity().getCount("ababaa"));
    }
}
