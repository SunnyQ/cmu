package myUDF;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;

import org.apache.pig.EvalFunc;
import org.apache.pig.data.BagFactory;
import org.apache.pig.data.DataBag;
import org.apache.pig.data.Tuple;
import org.apache.pig.data.TupleFactory;

public class MessageUpdate extends EvalFunc<DataBag> {

  @Override
  public DataBag exec(Tuple input) throws IOException {
    DataBag output = BagFactory.getInstance().newDefaultBag();
    List<Double> temps = new ArrayList<Double>();
    List<Double> saved_prior = new ArrayList<Double>();
    Hashtable<String, List<Double>> h = new Hashtable<String, List<Double>>();
    List<List<Double>> phi = getPhi();

    if (input == null || input.size() == 0)
      return null;

    Iterator<Tuple> iter = ((DataBag) input.get(1)).iterator();
    for (int i = 3; i < input.size(); i++) {
      saved_prior.add((Double) input.get(i));
    }
    while (iter.hasNext()) {
      Tuple curTuple = iter.next();
      List<Double> curList = new ArrayList<Double>();

      for (int i = 2; i < curTuple.size(); i++) {
        if (temps.size() <= i - 2)
          temps.add(1.0);
        if (temps.get(i - 2) < Math.pow(10, -130)) {
          temps.set(i - 2, temps.get(i - 2) * Math.pow(10, 129));
        }
        temps.set(i - 2, temps.get(i - 2) * ((Double) curTuple.get(i)));
        curList.add((Double) curTuple.get(i));
      }

      h.put(curTuple.get(1).toString(), curList);
    }

    for (String did : h.keySet()) {
      Tuple inner_output = TupleFactory.getInstance().newTuple();
      int numStates = saved_prior.size();
      List<Double> outm = new ArrayList<Double>(numStates);

      for (int u = 0; u < numStates; u++) {
        outm.add(0.0);
        for (int v = 0; v < numStates; v++) {
          outm.set(u,
                  outm.get(u)
                          + (saved_prior.get(v) * phi.get(v).get(u) * temps.get(v) / h.get(did)
                                  .get(v)));
        }
      }

      inner_output.append(did);
      inner_output.append(input.get(0));

      double max = 0.0;
      for (int i = 0; i < numStates; i++) {
        max = Math.max(outm.get(i), max);
      }
      double coefficient = Math.pow(10, Math.abs(Math.floor(Math.log10(max))) - 1);
      
      for (int i = 0; i < numStates; i++) {
        inner_output.append(outm.get(i) * coefficient);
      }
      output.add(inner_output);
    }

    return output;
  }

  List<List<Double>> getPhi() {
    List<List<Double>> phi = new ArrayList<List<Double>>();
    String curLine = null;
    BufferedReader br = null;

    try {
      br = new BufferedReader(new FileReader("input/propa_matrix"));
      while ((curLine = br.readLine()) != null) {
        List<Double> curPhi = new ArrayList<Double>();
        for (String e : curLine.split("\t"))
          curPhi.add(Double.parseDouble(e));
        phi.add(curPhi);
      }
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      // TODO Auto-generated catch block
      e.printStackTrace();
    } finally {
      if (br != null)
        try {
          br.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
    }
    return phi;
  }
}
