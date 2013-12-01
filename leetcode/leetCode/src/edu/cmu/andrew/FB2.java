package edu.cmu.andrew;

/**
 * 1->2->3->4->5->6->7 to 1->7->2->6->3->5->4, no extra space
 * @author Kobe
 *
 */
public class FB2 {
    private class Node {
        Node next;
        int val;
        public Node(int num) {
            val = num;
        }
    }
    
    public Node changeOrder2(Node head) {
        int size = 0;
        for (Node curNode = head; curNode != null; curNode = curNode.next) {
            size ++;
        }
        int mid = size / 2;
        if (mid == 0) {
            return head;
        }
        Node secondPart = head;
        for (int i = 0; i < mid; i ++, secondPart = secondPart.next);
        Node firstPart = reverseList(head, mid);
        Node mergePoint = (size % 2 == 1) ? secondPart.next : secondPart;
        for (int i = 0; i < mid; i ++) {
            Node mergeNext = mergePoint.next;
            Node firstNext = firstPart.next;
            mergePoint.next = firstPart;
            firstPart.next = mergeNext;
            mergePoint = mergeNext;
            firstPart = firstNext;
        }
        return reverseList(secondPart, size);
    }
    
    private Node reverseList(Node head, int num) {
        Node prev = null;
        Node cur = head;
        int counter = 0;
        while (cur != null && counter < num) {
            Node next = cur.next;
            cur.next = prev;
            counter ++;
            if (counter < num) {
                prev = cur;
                cur = next;
            }
        }
        return cur;
    }
    
    public Node changeOrder(Node head) {
        if (head == null) return head;
        Node curNode = head;
        Node lastNode = removeLastNode(curNode);
        while (lastNode != null) {
            lastNode.next = curNode.next;
            curNode.next = lastNode;
            curNode = lastNode.next;
            lastNode = removeLastNode(curNode);
        }
        return head;
    }
    
    private Node removeLastNode(Node node) {
        if (node == null || node.next == null) return null;
        for (; node.next.next != null; node = node.next);
        Node last = node.next;
        node.next = null;
        return last;
    }
    
    public void printAllNode(Node head) {
        Node curNode = head;
        while (curNode != null) {
            System.out.println(curNode.val);
            curNode = curNode.next;
        }
    }
    
    public static void main(String[] args) {
        FB2 fb = new FB2();
        Node head = fb.new Node(1);
        Node curNode = head;
        for (int i = 2; i < 8; i ++) {
            curNode.next = fb.new Node(i);
            curNode = curNode.next;
        }
        fb.printAllNode(fb.changeOrder2(head));
    }
    
}
