import java.util.ArrayList;
import java.util.Stack;

public class PreOrder {
    public class TreeNode {
        int val;
        TreeNode left;
        TreeNode right;

        TreeNode(int x) {
            val = x;
        }
    }

    public ArrayList<Integer> preOrderTraversal(TreeNode root) {
        ArrayList<Integer> res = new ArrayList<Integer>();
        Stack<TreeNode> stack = new Stack<TreeNode>();
        if (root == null) {
            return res;
        }
        
        TreeNode curNode = root;
        while (curNode != null) {
            res.add(curNode.val);
            stack.push(curNode);
            curNode = curNode.left;
        }
        
        while (!stack.isEmpty()) {
            curNode = stack.pop();
            if (curNode.right != null) {
                curNode = curNode.right;
                while (curNode != null) {
                    res.add(curNode.val);
                    stack.push(curNode);
                    curNode = curNode.left;
                }
            }
        }
        
        return res;
    }
    
    public static void main(String[] args) {
        PreOrder po = new PreOrder();
        TreeNode root = po.new TreeNode(1);
        root.left = po.new TreeNode(2);
        root.right = po.new TreeNode(3);
        root.right.left = po.new TreeNode(4);
        root.right.left.right = po.new TreeNode(5);
        System.out.println(po.preOrderTraversal(root));
    }
}
