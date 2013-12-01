package edu.cmu.andrew;

import java.util.Arrays;

/**
 * There are two sorted arrays A and B of size m and n respectively. Find the median of the two sorted arrays. The
 * overall run time complexity should be O(log (m+n)).
 * 
 * @author Kobe
 * 
 */
public class MedianOfTwoSortedArray {
    public double findMedianSortedArrays(int A[], int B[]) {
        // Start typing your Java solution below
        // DO NOT write main() function

        boolean isOdd = (A.length + B.length) % 2 == 1;
        if (isOdd) {
            return getKthLargest(A, B, (A.length + B.length) / 2 + 1);
        } else {
            return (getKthLargest(A, B, (A.length + B.length) / 2) + getKthLargest(A, B, (A.length + B.length) / 2 + 1)) / 2.0;
        }

    }

    /**
     * 
     * @param A
     * @param B
     * @param k
     *            represents the kth largest value
     * @return
     */
    private double getKthLargest(int A[], int B[], int k) {
        // always assume a.length < b.length
        if (A.length > B.length) {
            return getKthLargest(B, A, k);
        }

        // kth largest among all equals to kth largest in B.
        if (A.length == 0) {
            return B[k - 1];
        }

        // return smallest
        if (k == 1) {
            return Math.min(A[0], B[0]);
        }

        // create two pointers
        int i = Math.min(k / 2, A.length);
        int j = k - i;

        if (A[i - 1] < B[j - 1]) {
            return getKthLargest(Arrays.copyOfRange(A, i, A.length), B, k - i);
        } else if (A[i - 1] > B[j - 1]) {
            return getKthLargest(A, Arrays.copyOfRange(B, j, B.length), k - j);
        } else {
            return A[i - 1];
        }
    }

}