package myUDF;

import java.io.IOException;

import org.apache.pig.EvalFunc;
import org.apache.pig.data.Tuple;
import org.apache.pig.data.TupleFactory;

public class MessageConverge extends EvalFunc<Tuple> {

  @Override
  public Tuple exec(Tuple input) throws IOException {
    int size = input.size() / 2;
    double thershold = 0.01;
    Tuple output = TupleFactory.getInstance().newTuple();

    for (int i = 2; i < size; i++) {
      double prevVal = (Double) input.get(i);
      double updatedVal = (Double) input.get(size + i);
      if (Math.abs(prevVal - updatedVal) <= thershold) {
        output.append("done");
      }
    }

    return output;
  }
}
