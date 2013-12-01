package edu.cmu.andrew.mobileapp4.async;

import java.io.File;

import twitter4j.StatusUpdate;
import twitter4j.Twitter;
import twitter4j.TwitterException;
import twitter4j.TwitterFactory;
import twitter4j.auth.AccessToken;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.widget.Toast;

public class TwitterUpload extends AsyncTask<Void, Void, Boolean> {

  private final String tokenKey;

  private final String tokenVal;

  private final String consumerKey;

  private final String consumerVal;

  private Context context;

  private String statusMsg;

  private File photoTaken;

  private ProgressDialog progressDialog;

  public TwitterUpload(Context context, String msg, File photoTaken) {
    this.tokenKey = "124957012-D0x4NxDnkC0VLNHixFOTzvqjlLHCbvzFmd0rkm1k";
    this.tokenVal = "tzqufd3V5pLKpm0P67t3W5H8ewvzpqLNhJPoFAQgag";
    this.consumerKey = "UOhx7YcjBU2EkFwQfU6iw";
    this.consumerVal = "yq3WT4zwUJnifhnLJtCf6Qb5S8qAlQCWwI6PW0akE";
    this.context = context;
    this.statusMsg = msg;
    this.photoTaken = photoTaken;
  }

  @Override
  protected Boolean doInBackground(Void... params) {
    try {
      TwitterFactory factory = new TwitterFactory();
      Twitter twitter = factory.getInstance();
      twitter.setOAuthConsumer(consumerKey, consumerVal);
      twitter.setOAuthAccessToken(new AccessToken(tokenKey, tokenVal));
      StatusUpdate status = new StatusUpdate(statusMsg);
      if (photoTaken != null && photoTaken.exists())
        status.setMedia(photoTaken);
      twitter.updateStatus(status);
      return true;
    } catch (TwitterException e) {
      return false;
    }
  }

  @Override
  protected void onPreExecute() {
    super.onPreExecute();
    progressDialog = ProgressDialog.show(context, "Updating...", "Please wait...");
  }

  @Override
  protected void onPostExecute(Boolean result) {
    super.onPostExecute(result);
    if (result) {
      Toast.makeText(context, "Successfully updated the status!", Toast.LENGTH_LONG).show();
    } else {
      Toast.makeText(context, "Error: Internet Connection failure", Toast.LENGTH_LONG).show();
    }
    progressDialog.dismiss();
    ((Activity) context).finish();
  }

}
