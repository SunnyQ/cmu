package edu.cmu.andrew;

import java.util.Arrays;

public class BuildTreeInOrderPostOrder {
    public class TreeNode {
        int val;
        TreeNode left;
        TreeNode right;

        TreeNode(int x) {
            val = x;
        }
    }

    public TreeNode buildTree(int[] inorder, int[] postorder) {
        if (inorder.length == 0 || postorder.length == 0)
            return null;
        int rootVal = postorder[postorder.length - 1];
        TreeNode root = new TreeNode(rootVal);
        int rootPos = 0;
        for (int i = 0; i < inorder.length; i++) {
            if (rootVal == inorder[i]) {
                rootPos = i;
                break;
            }
        }
        root.left = buildTree(Arrays.copyOfRange(inorder, 0, rootPos), Arrays.copyOfRange(postorder, 0, rootPos));
        root.right = buildTree(Arrays.copyOfRange(inorder, rootPos + 1, inorder.length),
                Arrays.copyOfRange(postorder, rootPos, postorder.length - 1));
        return root;
    }

    public static void main(String[] args) {
        int[] inorder = { 1, 2 };
        int[] postorder = { 2, 1 };
        new BuildTreeInOrderPostOrder().buildTree(inorder, postorder);
    }

}
