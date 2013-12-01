package edu.cmu.andrew;

public class OccurrencesNumSortedArray {
    private int[] a;
    private int x;

    public OccurrencesNumSortedArray(int[] a, int x) {
        this.a = a;
        this.x = x;
    }

    public int findOccurrencesInSortedArray() {
        // search first occurrence
        int low = 0, high = a.length - 1;
        while (low < high) {
            int middle = (high + low) / 2;
            if (a[middle] < x) {
                // the first occurrence must come after index middle, if any
                low = middle + 1;
            } else if (a[middle] > x) {
                // the first occurrence must come before index middle if at all
                high = middle - 1;
            } else {
                // found an occurrence, it may be the first or not
                high = middle;
            }
        }
        if (high < low || a[low] != x) {
            // that means no occurrence
            return 0;
        }
        // remember first occurrence
        int first = low;
        // search last occurrence, must be between low and a.length-1 inclusive
        high = a.length - 1;
        // now, we always have a[low] == x and high is the index of the last occurrence or later
        while (low < high) {
            // bias middle towards high now
            int middle = (high + low) / 2;
            if (a[middle] > x) {
                // the last occurrence must come before index middle
                high = middle - 1;
            } else {
                // last known occurrence
                low = middle;
            }
        }
        // high is now index of last occurrence
        return (high - first + 1);
    }
    
    public static void main(String[] args) {
        int[] a = {0, 1, 2, 2, 2, 3, 4};
        System.out.println(new OccurrencesNumSortedArray(a, 2).findOccurrencesInSortedArray());
    }
}
