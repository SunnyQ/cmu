import java.util.HashSet;


public class StringPermutation {

    public void printAllPermutations(String a) {
        HashSet<String> res = new HashSet<String>();
        generateAllPermutations("", a, res);
        for (String r : res) {
            System.out.println(r);
        }
        System.out.println(res.size());
    }
    
    private void generateAllPermutations(String prefix, String rest, HashSet<String> res) {
        if (rest.length() == 0) {
            res.add(prefix);
            return;
        }
        
        for (int i = 0; i < rest.length(); i ++) {
            generateAllPermutations(prefix + rest.charAt(i), rest.substring(0, i) + rest.substring(i + 1), res);
        }
    }
    
    public static void main(String[] args) {
        new StringPermutation().printAllPermutations("ABCA");
    }
    
}
