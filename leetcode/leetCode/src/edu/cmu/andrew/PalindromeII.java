package edu.cmu.andrew;

public class PalindromeII {

  public int minCut(String s) {
    // Start typing your Java solution below
    // DO NOT write main() function
    int leng = s.length();
    boolean[][] isPal = new boolean[leng][leng];

    int[] dp = new int[leng + 1]; // dp[i] indicates the min cut of
                                  // s[i...leng - 1]
    for (int i = 0; i != leng + 1; ++i) {
      dp[i] = leng - 1 - i; // since a single char is always a palindrome, the
                            // max cut string s needs is (s.length() - 1). Note
                            // that dp[leng] = -1, because it is used below as
                            // dp[i] = Math.min(dp[i], 1 + dp[j + 1]);
    }

    for (int i = leng - 2; i >= 0; --i) {
      for (int j = i; j < leng; ++j) {
        if (s.charAt(i) == s.charAt(j) && (j <= i + 2 || (isPal[i + 1][j - 1]))) {
          isPal[i][j] = true;
          dp[i] = Math.min(dp[i], 1 + dp[j + 1]);
        }
      }
    }

    return dp[0];
  }

  public static void main(String[] args) {
    PalindromeII instance = new PalindromeII();
    System.out.println(instance.minCut("aab"));
  }
}
