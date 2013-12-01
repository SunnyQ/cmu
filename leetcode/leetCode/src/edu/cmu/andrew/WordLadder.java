package edu.cmu.andrew;

import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;

public class WordLadder {

    public int ladderLength(String start, String end, HashSet<String> dict) {
        Queue<String> queue = new LinkedList<String>();
        queue.add(start);
        dict.remove(start);
        int steps = 1;
        
        LinkedList<String> backUps = new LinkedList<String>();
        while (!queue.isEmpty()) {
            String target = queue.poll();
            if (isAdjacent(target, end)) {
                return ++steps;
            }
            Iterator<String> dictItor = dict.iterator();
            while (dictItor.hasNext()) {
                String dictItem = dictItor.next();
                if (isAdjacent(dictItem, target)) {
                    backUps.add(dictItem);
                }
            }
            if (queue.isEmpty()) {
                dict.removeAll(backUps);
                queue = backUps;
                backUps = new LinkedList<String>();
                steps ++;
            }
        }
        
        return 0;
    }
    
    private boolean isAdjacent(String a, String b) {
        int count = 0;
        for (int i = 0; i < a.length(); i ++) {
            if (a.charAt(i) != b.charAt(i)) {
                count ++;
            }
        }
        return count == 1;
    }

    public static void main(String[] args) {
        HashSet<String> dict = new HashSet<String>();
        dict.add("lest");
        dict.add("leet");
        dict.add("lose");
        dict.add("code");
        dict.add("lode");
        dict.add("robe");
        dict.add("lost");
        System.out.println(new WordLadder().ladderLength("leet", "code", dict));
    }
}
