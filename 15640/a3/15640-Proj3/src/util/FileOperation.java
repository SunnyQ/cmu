package util;

import java.io.File;
import java.io.IOException;

/**
 * Class: FileOperation.java
 * 
 * A utility tool to manipulate file operation more convenient.
 * 
 * @author Yuan Gu
 * 
 */
public class FileOperation {

  /**
   * mkdir Wrapper for easy use.
   * 
   * @param path
   * @return
   */
  public static boolean createDir(String path) {
    File file = new File(path);
    if (!file.exists()) {
      if (file.mkdir()) {
        return true;
      } else {
        return false;
      }
    } else {
      if (!file.isDirectory()) {
        return false;
      }
    }
    return true;
  }

  /**
   * Rename a file.
   * 
   * @param filePath
   * @param filePathRenamed
   * @param deleteIfExist
   * @throws IOException
   */
  public static void renameFile(String filePath, String filePathRenamed, boolean deleteIfExist)
          throws IOException {
    File file = new File(filePath);
    File fileRenamed = new File(filePathRenamed);
    if (fileRenamed.exists()) {
      if (!deleteIfExist)
        throw new IOException("Temporary file " + filePathRenamed + " exists.");
      else {
        if (!fileRenamed.delete())
          throw new IOException("Cannot delete existing temporary file " + filePathRenamed + ".");
      }
    }

    boolean success = file.renameTo(fileRenamed);
    if (!success) {
      throw new IOException("Fail to rename temporary file " + filePath + " to " + filePathRenamed);
    }
  }

  /**
   * Delete a file.
   * 
   * @param filePath
   * @throws IOException
   */
  public static void deleteFile(String filePath) throws IOException {
    File file = new File(filePath);
    if (file.exists()) {
      if (!file.delete())
        throw new IOException("Cannot delete file " + filePath + ".");
    }
  }
}
