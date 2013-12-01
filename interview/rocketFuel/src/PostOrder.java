import java.util.ArrayList;
import java.util.Stack;

public class PostOrder {

    public class TreeNode {
        int val;
        TreeNode left;
        TreeNode right;

        TreeNode(int x) {
            val = x;
        }
    }

    public ArrayList<Integer> postorderTraversal(TreeNode root) {
        // Note: The Solution object is instantiated only once and is reused by each test case.
        ArrayList<Integer> res = new ArrayList<Integer>();
        Stack<TreeNode> stack = new Stack<TreeNode>();
        if (root == null) {
            return res;
        } 
        
        TreeNode curNode = root;
        while (curNode != null) {
            stack.push(curNode);
            curNode = curNode.left;
        }
        
        TreeNode prev = null;
        while (!stack.isEmpty()) {
            curNode = stack.peek();
            if (curNode.right != null && prev != curNode.right) {
                curNode = curNode.right;
                while (curNode != null) {
                    stack.push(curNode);
                    curNode = curNode.left;
                }
            } else {
                prev = curNode;
                res.add(stack.pop().val);
            }
        }
        
        return res;
    }
    
    public static void main(String[] args) {
        PostOrder po = new PostOrder();
        TreeNode root = po.new TreeNode(1);
        root.left = po.new TreeNode(2);
        root.right = po.new TreeNode(3);
        root.right.left = po.new TreeNode(4);
        root.right.left.right = po.new TreeNode(5);
        System.out.println(po.postorderTraversal(root));
    }
}
