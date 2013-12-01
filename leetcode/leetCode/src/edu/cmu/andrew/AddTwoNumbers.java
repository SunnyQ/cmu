package edu.cmu.andrew;

/**
 * You are given two linked lists representing two non-negative numbers. The digits are stored in
 * reverse order and each of their nodes contain a single digit. Add the two numbers and return it
 * as a linked list.
 * 
 * Input: (2 -> 4 -> 3) + (5 -> 6 -> 4) Output: 7 -> 0 -> 8
 * 
 * @author Kobe
 * 
 */
public class AddTwoNumbers {

  public class ListNode {
    int val;

    ListNode next;

    ListNode(int x) {
      val = x;
      next = null;
    }
  }

  public ListNode addTwoNumbers(ListNode l1, ListNode l2) {
    // Start typing your Java solution below
    // DO NOT write main() function
    int carry = 0;
    ListNode dummy = new ListNode(0);

    ListNode curNode = dummy;
    while (l1 != null || l2 != null || carry > 0) {
      int sum = (l1 == null ? 0 : l1.val) + (l2 == null ? 0 : l2.val) + carry;
      curNode.next = new ListNode(sum % 10);
      carry = sum / 10;
      if (l1 != null)
        l1 = l1.next;
      if (l2 != null)
        l2 = l2.next;
      curNode = curNode.next;
    }
    return dummy.next;
  }
}
