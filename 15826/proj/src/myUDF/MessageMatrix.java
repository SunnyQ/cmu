package myUDF;

import java.io.IOException;

import org.apache.pig.EvalFunc;
import org.apache.pig.data.Tuple;
import org.apache.pig.data.TupleFactory;

public class MessageMatrix extends EvalFunc<Tuple> {
  @Override
  public Tuple exec(Tuple input) throws IOException {
    Tuple output = TupleFactory.getInstance().newTuple();

    if (input == null || input.size() == 0)
      return null;
    
    for (int i = 0; i < input.size(); i++) {
      if (i < 2)
        output.append(input.get(i));
      else if (i > 2)
        output.append(1.0 / (input.size() - 3));
    }
    
    return output;
  }
}
