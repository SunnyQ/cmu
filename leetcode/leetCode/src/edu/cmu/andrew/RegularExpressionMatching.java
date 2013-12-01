package edu.cmu.andrew;

public class RegularExpressionMatching {

    public boolean isMatch(String s, String p) {
        if (p.length() == 0) {
            return s.length() == 0;
        }
        
        if ((p.length() > 1 && p.charAt(1) != '*') || p.length() == 1) {
            if (s.length() == 0 || (p.charAt(0) != s.charAt(0) && p.charAt(0) != '.')) {
                return false;
            }
            return isMatch(s.substring(1), p.substring(1));
        }
        
        int i = 0;
        while (i < s.length() && (s.charAt(i) == p.charAt(0) || p.charAt(0) == '.')) {
            if (isMatch(s.substring(i + 1), p.substring(2))) {
                return true;
            }
            i ++;
        }
        return isMatch(s.substring(i), p.substring(2));
    }
    
    public static void main(String[] args) {
        RegularExpressionMatching regex = new RegularExpressionMatching();
        System.out.println(regex.isMatch("ab", ".ba*"));
    }
}
