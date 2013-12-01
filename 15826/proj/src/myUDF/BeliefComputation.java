package myUDF;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.pig.EvalFunc;
import org.apache.pig.data.DataBag;
import org.apache.pig.data.Tuple;
import org.apache.pig.data.TupleFactory;

public class BeliefComputation extends EvalFunc<Tuple> {

  @Override
  public Tuple exec(Tuple input) throws IOException {
    Tuple output = TupleFactory.getInstance().newTuple();
    List<Double> b = new ArrayList<Double>();

    if (input == null || input.size() == 0)
      return null;

    for (int i = 3; i < input.size(); i++)
      b.add((Double) input.get(i));

    Iterator<Tuple> iter = ((DataBag) input.get(1)).iterator();
    while (iter.hasNext()) {
      Tuple curTuple = iter.next();
      for (int i = 2; i < curTuple.size(); i++)
        b.set(i - 2, b.get(i - 2) * (Double) curTuple.get(i));
    }

    output.append(input.get(0));
    for (int i = 0; i < b.size(); i++)
      output.append(b.get(i));
    return output;
  }
}
