package edu.cmu.andrew.mobileapp4;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings.Secure;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;
import edu.cmu.andrew.mobileapp4.async.TwitterUpload;

public class ComposeActivity extends Activity implements OnClickListener, OnTouchListener {

  private ImageView image;

  private Button publishButton;

  private EditText editText;

  private File photoTaken;

  private LinearLayout layout;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_compose);

    layout = (LinearLayout) findViewById(R.id.compose_layout);
    layout.setOnTouchListener(this);

    image = (ImageView) findViewById(R.id.image);
    publishButton = (Button) findViewById(R.id.publish);

    editText = (EditText) findViewById(R.id.status);
    editText.setFocusable(true);

    if (getIntent().getExtras() != null && getIntent().getExtras().containsKey("photo")) {
      photoTaken = new File(getIntent().getExtras().getString("photo"));
      Bitmap srcImg = BitmapFactory.decodeFile(photoTaken.getAbsolutePath());
      image.setImageBitmap(Bitmap.createScaledBitmap(srcImg, 320, 320, false));
    } else {
      photoTaken = null;
    }

    publishButton.setClickable(true);
    publishButton.setOnClickListener(this);

    try {
      editText.setText(getRequiredMsg());
    } catch (NameNotFoundException e) {
      Toast.makeText(this, "Error: " + e.toString(), Toast.LENGTH_LONG).show();
    }
  }

  @Override
  public void onClick(View v) {
    if (v == publishButton) {
      new TwitterUpload(this, editText.getText().toString(), photoTaken).execute();
    }
  }

  @Override
  public boolean onTouch(View v, MotionEvent event) {
    if (v != editText) {
      InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
      imm.hideSoftInputFromWindow(v.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
    }
    return false;
  }

  private String getRequiredMsg() throws NameNotFoundException {
    String at = "@MobileApp4";
    String andrewID = "yksun";
    String dateTime = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss", Locale.US).format(new Date());
    String deviceID = Secure.getString(getContentResolver(), Secure.ANDROID_ID);
    String model = "Android " + Build.VERSION.RELEASE;

    return at + " /" + andrewID + "/ " + dateTime + " <UIDevice: " + deviceID + "> " + model;
  }

}
