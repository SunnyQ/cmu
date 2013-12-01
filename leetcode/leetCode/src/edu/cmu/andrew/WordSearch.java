package edu.cmu.andrew;

public class WordSearch {
    public boolean findTarget(String subWord, char[][] board, int i, int j, boolean[][] visited) {
        if (subWord.length() == 0) {
            return true;
        }

        if (i < 0 || i >= board.length || j < 0 || j >= board[0].length || visited[i][j]
                || board[i][j] != subWord.charAt(0)) {
            return false;
        }

        visited[i][j] = true;

        boolean res = findTarget(subWord.substring(1), board, i - 1, j, visited)
                || findTarget(subWord.substring(1), board, i, j - 1, visited)
                || findTarget(subWord.substring(1), board, i + 1, j, visited)
                || findTarget(subWord.substring(1), board, i, j + 1, visited);
        
        visited[i][j] = false;
        return res;

    }

    public boolean exist(char[][] board, String word) {
        // Note: The Solution object is instantiated only once and is reused by each test case.
        if (word == null)
            return false;
        boolean[][] visited = new boolean[board.length][board[0].length];
        for (int i = 0; i < board.length; i++) {
            for (int j = 0; j < board[i].length; j++) {
                if (board[i][j] == word.charAt(0) && findTarget(word, board, i, j, visited)) {
                    return true;
                }
            }
        }
        return false;
    }

    public static void main(String[] args) {
        char[][] board = new char[1][2];
        board[0][0] = 'a';
        board[0][1] = 'a';
        System.out.println(new WordSearch().exist(board, "aaa"));
    }
}
