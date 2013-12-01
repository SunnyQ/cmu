import java.util.ArrayList;

public class Permutation {

    private int n;
    private ArrayList<Integer> possibilities;
    private ArrayList<Integer> solution;

    public Permutation(int n) {
        this.n = n;
        possibilities = new ArrayList<Integer>();
        solution = new ArrayList<Integer>();
        
        for (int i = 1; i <= n; i++) {
            int pow = i * i;
            if (pow <= 2 * n - 1 && pow >= 3) {
                possibilities.add(pow);
            }
        }

        for (int i = 1; i <= n; i++) {
            solution.add(i);
            printAllPermutations();
            solution.remove(solution.size() - 1);
        }
    }

    public void printAllPermutations() {
        if (solution.size() == n) {
            System.out.println(solution);
            return;
        }

        int last = solution.get(solution.size() - 1);
        for (int i = 0; i < possibilities.size(); i++) {
            int target = possibilities.get(i) - last;
            if (target >= 1 && target <= n && !solution.contains(target)) {
                solution.add(target);
                printAllPermutations();
                solution.remove(solution.size() - 1);
            }
        }
    }

    public static void main(String[] args) {
        new Permutation(16);
    }
}
