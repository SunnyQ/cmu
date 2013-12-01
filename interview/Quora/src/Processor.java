import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Quora - Nearby
 * Processor.java
 * Purpose: process the user input and output the result
 * Assume: the user input is in good format, may lack of exception handling due to time constraint
 * 
 * @author Yang Sun
 * @version 1.0 9/24/2013
 */
public class Processor {

    private final Map<Integer, Topic> topics;
    private final Map<Integer, Question> questions;

    public Processor() {
        this.topics = new HashMap<Integer, Topic>();
        this.questions = new HashMap<Integer, Question>();
    }

    /**
     * Process the topic related user input
     * @param numTopics number of topics in the input
     * @param br the bufferedReader stream
     * @throws IOException
     */
    public void processTopics(int numTopics, BufferedReader br) throws IOException {
        String line[];
        for (int i = 0; i < numTopics; i++) {
            line = br.readLine().trim().split(" +");
            Topic topic = new Topic(Integer.parseInt(line[0]), Double.parseDouble(line[1]), Double.parseDouble(line[2]));
            topics.put(topic.getId(), topic);
        }
    }

    /**
     * Process the question related user input
     * @param numQuestions number of questions in the input
     * @param br the bufferedReader stream
     * @throws IOException
     */
    public void processQuestions(int numQuestions, BufferedReader br) throws IOException {
        String line[];
        for (int i = 0; i < numQuestions; i++) {
            line = br.readLine().trim().split(" +");
            Question question = new Question(Integer.parseInt(line[0]));
            questions.put(question.getId(), question);

            for (int j = 0; j < Integer.parseInt(line[1]); j++) {
                question.addAssociatedTopic(topics.get(Integer.parseInt(line[2 + j])));
            }
        }
    }
    
    /**
     * Process the queries
     * @param numQueries number of the queries in the input
     * @param br the bufferedReader stream
     * @return the result String
     * @throws IOException
     */
    public String processQueries(int numQueries, BufferedReader br) throws IOException {
        StringBuilder sb = new StringBuilder();
        
        String line[];
        for (int i = 0; i < numQueries; i ++) {
            line = br.readLine().trim().split(" +");
            Location center = new Location(Double.parseDouble(line[2]), Double.parseDouble(line[3]));
            if (line[0].equals("t")) {
                sb.append(queryTopic(Integer.parseInt(line[1]), center));
            } else if (line[0].equals("q")) {
                sb.append(queryQuestion(Integer.parseInt(line[1]), center));
            }
            sb.append("\n");
        }
        return sb.toString();
    }
    
    /**
     * Process the topic type query
     * @param numRes number of the answers the user want
     * @param center the center point of the location
     * @return the result String
     */
    private String queryTopic(int numRes, Location center) {
        StringBuilder sb = new StringBuilder();
        List<Topic> allTopics = new ArrayList<Topic>(topics.values());
        Collections.sort(allTopics, new TopicComparator(center));
        for (int i = 0; i < numRes; i ++) {
            sb.append(allTopics.get(i).getId() + " ");
        }
        return sb.toString().trim();
    }
    
    /**
     * Process the question type queries
     * @param numRes number of the answers the user want
     * @param center the center point of the location
     * @return the result String
     */
    private String queryQuestion(int numRes, Location center) {
        StringBuilder sb = new StringBuilder();
        List<Question> questionList = new ArrayList<Question>(questions.values());
        Collections.sort(questionList, new QuestionComparator(center));
        for (int i = 0; i < numRes; i ++) {
            if (questionList.get(i).getAssociatedTopicsSize() != 0) {
                sb.append(questionList.get(i).getId() + " ");
            }
        }
        return sb.toString().trim();
    }

    public static void main(String[] args) {
        BufferedReader br = null;
        Processor processor = new Processor();
        try {
            br = new BufferedReader(new InputStreamReader(System.in));
            String[] line = br.readLine().trim().split(" +");
            processor.processTopics(Integer.parseInt(line[0]), br);
            processor.processQuestions(Integer.parseInt(line[1]), br);
            String output = processor.processQueries(Integer.parseInt(line[2]), br);
            System.out.println(output);
        } catch (IOException E) {
            System.out.println("The input is bad formatted");
        } finally {
            if (br != null) {
                try {
                    br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
