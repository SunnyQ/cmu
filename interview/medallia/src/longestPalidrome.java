/* Enter your code here. Read input from STDIN. Print output to STDOUT */

public class longestPalidrome {

    public longestPalidrome(String input) {
        String processedInput = preProcess(input);
        int[] pool = new int[processedInput.length()];
        int center = 0;
        int right = 0;

        for (int i = 1; i < processedInput.length() - 1; i++) {
            int iMirror = 2 * center - i;
            pool[i] = (right > i) ? Math.min(right - i, pool[iMirror]) : 0;

            while (processedInput.charAt(i + 1 + pool[i]) == processedInput.charAt(i - 1 - pool[i])) {
                pool[i]++;
            }

            if (i + pool[i] > right) {
                center = i;
                right = i + pool[i];
            }
        }

        int maxLength = 0;
        int centerIndex = 0;
        for (int i = 1; i < processedInput.length() - 1; i++) {
            if (pool[i] > maxLength) {
                maxLength = pool[i];
                centerIndex = i;
            }
        }

        int start = (centerIndex - 1 - maxLength) / 2;
        System.out.println(input.substring(start, start + maxLength));
    }

    private String preProcess(String input) {
        if (input.length() == 0) {
            return "^$";
        }
        StringBuilder ret = new StringBuilder("^");
        for (int i = 0; i < input.length(); i++) {
            ret.append("#");
            ret.append(input.substring(i, i + 1));
        }
        ret.append("#$");
        return ret.toString();
    }

    public static void main(String[] args) throws Exception {
        new longestPalidrome("I do step on no pets forever");
    }
}