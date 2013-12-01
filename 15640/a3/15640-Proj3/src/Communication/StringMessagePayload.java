package Communication;

import Communication.Message.MessagePayload;

/**
 * Class: ReducerTaskPayload.java
 * 
 * The Message Payload used to communicate between master and client for general purpose
 * 
 * @author Yang Sun
 * 
 */
public class StringMessagePayload extends MessagePayload {

  private static final long serialVersionUID = -391025298717555148L;

  private String msg;

  private String attr;

  public StringMessagePayload(String msg) {
    setMsg(msg);
  }

  public String getMsg() {
    return msg;
  }

  public void setMsg(String msg) {
    this.msg = msg;
  }

  public String getAttr() {
    return attr;
  }

  public void setAttr(String attr) {
    this.attr = attr;
  }
}