package edu.cmu.andrew;

/**
 * Given a string, find the length of the longest substring without repeating characters. For
 * example, the longest substring without repeating letters for "abcabcbb" is "abc", which the
 * length is 3. For "bbbbb" the longest substring is "b", with the length of 1.
 * 
 * @author Kobe
 * 
 */
public class LongestSubstringWithoutRepeatingCharacters {
  public int lengthOfLongestSubstring(String s) {
    // Start typing your Java solution below
    // DO NOT write main() function
    boolean[] exists = new boolean[256];

    int maxLen = 0;
    int i = 0;
    int j = 0;
    while (j < s.length()) {
      if (exists[s.charAt(j)]) {
        maxLen = Math.max(maxLen, j - i);
        while (s.charAt(i) != s.charAt(j))
          exists[s.charAt(i++)] = false;
        i++;
      }
      exists[s.charAt(j)] = true;
      j++;
    }

    return Math.max(maxLen, j - i);
  }
}
