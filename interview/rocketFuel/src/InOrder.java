import java.util.ArrayList;
import java.util.Stack;

public class InOrder {

    public class TreeNode {
        int val;
        TreeNode left;
        TreeNode right;

        TreeNode(int x) {
            val = x;
        }
    }

    public ArrayList<Integer> inorderTraversal(TreeNode root) {
        // Note: The Solution object is instantiated only once and is reused by each test case.
        ArrayList<Integer> res = new ArrayList<Integer>();
        if (root == null) {
            return res;
        }
        Stack<TreeNode> stack = new Stack<TreeNode>();
        TreeNode curNode = root;
        while (curNode != null) {
            stack.push(curNode);
            curNode = curNode.left;
        }

        while (!stack.isEmpty()) {
            curNode = stack.pop();
            res.add(curNode.val);
            if (curNode.right != null) {
                curNode = curNode.right;
                while (curNode != null) {
                    stack.push(curNode);
                    curNode = curNode.left;
                }
            }
        }

        return res;
    }

    public static void main(String[] args) {
        InOrder io = new InOrder();
        TreeNode root = io.new TreeNode(1);
        root.left = io.new TreeNode(2);
        root.right = io.new TreeNode(3);
        root.right.left = io.new TreeNode(4);
        root.right.left.right = io.new TreeNode(5);
        System.out.println(io.inorderTraversal(root));
    }
}
