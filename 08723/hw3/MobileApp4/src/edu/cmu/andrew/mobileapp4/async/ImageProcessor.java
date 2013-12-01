package edu.cmu.andrew.mobileapp4.async;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import edu.cmu.andrew.mobileapp4.ComposeActivity;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.os.AsyncTask;

public class ImageProcessor extends AsyncTask<Void, Void, Void> {

  private String imagePath;

  private Context context;

  private ProgressDialog progressDialog;

  public ImageProcessor(Context context, String imagePath) {
    this.imagePath = imagePath;
    this.context = context;
  }

  @Override
  protected Void doInBackground(Void... params) {
    try {
      ExifInterface exif = new ExifInterface(imagePath);
      int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION,
              ExifInterface.ORIENTATION_NORMAL);
      int rotation = 0;
      switch (orientation) {
        case ExifInterface.ORIENTATION_ROTATE_270:
          rotation += 90;
        case ExifInterface.ORIENTATION_ROTATE_180:
          rotation += 90;
        case ExifInterface.ORIENTATION_ROTATE_90:
          rotation += 90;
      }
      rotateImage(imagePath, rotation);
    } catch (IOException e) {
      e.printStackTrace();
    }
    return null;
  }

  @Override
  protected void onPreExecute() {
    super.onPreExecute();
    progressDialog = ProgressDialog.show(context, "Saving...", "Please wait...");
  }

  @Override
  protected void onPostExecute(Void result) {
    super.onPostExecute(result);
    progressDialog.dismiss();

    Intent intentMain = new Intent(context, ComposeActivity.class);
    intentMain.putExtra("photo", imagePath);
    context.startActivity(intentMain);
  }

  public void rotateImage(String imagePath, int rotation) throws IOException {
    Matrix matrix = new Matrix();
    matrix.postRotate(rotation);

    Bitmap bm = BitmapFactory.decodeFile(imagePath);
    Bitmap out = Bitmap.createBitmap(bm, 0, 0, bm.getWidth(), bm.getHeight(), matrix, true);

    File outfile = new File(imagePath);
    FileOutputStream fOut = new FileOutputStream(outfile);
    out.compress(Bitmap.CompressFormat.JPEG, 100, fOut);
    fOut.flush();
    fOut.close();
  }

}
