package edu.cmu.andrew.mobileapp4;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Locale;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.StrictMode;
import android.os.StrictMode.ThreadPolicy;
import android.os.StrictMode.ThreadPolicy.Builder;
import android.provider.MediaStore;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import edu.cmu.andrew.mobileapp4.async.ImageProcessor;

public class MainActivity extends Activity implements OnClickListener {

  private int REQUEST_CAMERA = 115;

  private Button photoButton;

  private Button composeButton;

  private String photoPath;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);

    if (Build.VERSION.SDK_INT > 9) {
      ThreadPolicy policy = new Builder().permitAll().build();
      StrictMode.setThreadPolicy(policy);
    }

    photoButton = (Button) findViewById(R.id.photo_btn);
    photoButton.setClickable(true);
    photoButton.setOnClickListener(this);

    composeButton = (Button) findViewById(R.id.compose_btn);
    composeButton.setClickable(true);
    composeButton.setOnClickListener(this);

  }

  @Override
  protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    if (requestCode == REQUEST_CAMERA && resultCode == RESULT_OK) {
      new ImageProcessor(this, photoPath).execute();
    }
  }

  public static final String generateDCIMFilename() {
    Calendar cal = Calendar.getInstance();
    SimpleDateFormat date = new SimpleDateFormat("yyyyMMdd", Locale.US);
    SimpleDateFormat time = new SimpleDateFormat("Hmmss", Locale.US);
    return "IMG_" + date.format(cal.getTime()) + "_" + time.format(cal.getTime()) + ".jpg";
  }

  @Override
  public void onClick(View v) {
    if (v == photoButton) {
      Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
      File photoFile = new File(Environment.getExternalStorageDirectory() + File.separator
              + Environment.DIRECTORY_PICTURES, generateDCIMFilename());
      photoPath = photoFile.getAbsolutePath();
      intent.putExtra(MediaStore.EXTRA_OUTPUT, Uri.fromFile(photoFile));
      startActivityForResult(intent, REQUEST_CAMERA);
    }

    else if (v == composeButton) {
      Intent intent = new Intent(this, ComposeActivity.class);
      startActivity(intent);
    }
  }
}
