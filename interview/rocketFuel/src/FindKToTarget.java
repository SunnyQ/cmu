import java.util.Arrays;

/**
 * for example: [4,6,87,93,46,8] = 244 50 = k target [4,6,50,50,46,8] = 164
 * 
 * @author Kobe
 * 
 */
public class FindKToTarget {
    private int findK(int[] input, int target) {
        if (input.length == 0) {
            if (target != 0) return -1;
            return 0;
        }
        Arrays.sort(input);
        int sum = 0;
        for (int i = 0; i < input.length; i ++) {
            int size = input.length - i;
            int goal = target - sum;
            if (goal % size == 0) {
                int k = goal / size;
                if (k < input[i]) {
                    return k;
                }
            }
            sum += input[i];
        }
        return -1;
    }
    
    public static void main(String[] args) {
        int[] input = {4, 6, 87, 93, 46, 8};
        System.out.println(new FindKToTarget().findK(input, 164));
    }
}
