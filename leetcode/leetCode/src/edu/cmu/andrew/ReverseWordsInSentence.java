package edu.cmu.andrew;

public class ReverseWordsInSentence {
    public String reverseWords(String sentence) {
        char[] charArr = sentence.toCharArray();
        reverseString(charArr, 0, charArr.length - 1);
        int i = 0;
        while (i < charArr.length) {
            if (charArr[i] == ' ') {
                i++;
            } else {
                int j;
                for (j = i; j < charArr.length; j++) {
                    if (charArr[j] == ' ') {
                        reverseString(charArr, i, j - 1);
                        break;
                    }
                }
                i = j + 1;
            }
        }
        return new String(charArr);
    }

    private void reverseString(char[] input, int i, int j) {
        int mid = (j + i) / 2;
        while (i <= mid) {
            if (input[i] != input[j]) {
                input[i] ^= input[j];
                input[j] ^= input[i];
                input[i] ^= input[j];
            }
            i ++;
            j --;
        }
    }

    public static void main(String[] args) {
        System.out.println(new ReverseWordsInSentence().reverseWords("i like this program very much"));
    }
}
