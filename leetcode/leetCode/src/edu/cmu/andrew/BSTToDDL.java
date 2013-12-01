package edu.cmu.andrew;

public class BSTToDDL {
    private class Node {
        Node left;
        Node right;
    }

    public Node treeToList(Node root) {
        if (root == null) {
            return null;
        }
        Node leftTree = treeToList(root.left);
        Node rightTree = treeToList(root.right);
        Node head;
        if (leftTree == null) {
            head = root;
        } else {
            head = leftTree;
            leftTree.left.right = root;
            root.left = leftTree.left;
        }
        if (rightTree == null) {
            head.left = root;
            root.right = head;
        } else {
            head.left = rightTree.left.right;
            rightTree.left.right = head;
            rightTree.left = root;
        }
        return head;
    }
}
