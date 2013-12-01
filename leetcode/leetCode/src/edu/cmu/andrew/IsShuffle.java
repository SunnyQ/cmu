package edu.cmu.andrew;

/**
 * is shuffle: maintain left->right order
 * @author Kobe
 *
 */
public class IsShuffle {
    public boolean isShuffle(String a, String b, String c) {
        if (a == null || a.length() == 0) {
            return b.equals(c);
        }
        if (b == null || b.length() == 0) {
            return a.equals(c);
        }
        if (c == null || c.length() == 0) {
            return a.length() + b.length() == 0;
        }
        
        if (a.length() + b.length() != c.length()) {
            return false;
        }
        
        if (a.charAt(0) == c.charAt(0)) {
            return isShuffle(a.substring(1), b, c.substring(1));
        } 
        
        if (b.charAt(0) == c.charAt(0)) {
            return isShuffle(a, b.substring(1), c.substring(1));
        }
        
        return false;
    }
}
