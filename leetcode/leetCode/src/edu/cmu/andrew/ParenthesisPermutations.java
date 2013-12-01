package edu.cmu.andrew;

public class ParenthesisPermutations {

    public void permutations (int n) {
        generatePermutations(n, 0, "");
    }
    
    private void generatePermutations(int openBrackets, int closeBrackets, String s) {
        if (openBrackets == 0 && closeBrackets == 0) {
            System.out.println(s);
            return;
        }
        
        if (openBrackets > 0) {
            generatePermutations(openBrackets - 1, closeBrackets + 1, s + "{");
        }
        
        if (closeBrackets > 0) {
            generatePermutations(openBrackets, closeBrackets - 1, s + "}");
        }
    }
    
    public static void main(String[] args) {
        new ParenthesisPermutations().permutations(3);
    }
}
