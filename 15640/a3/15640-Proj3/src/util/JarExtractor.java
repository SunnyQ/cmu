package util;

import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

/**
 * Class: JarExtractor.java
 * 
 * A utility tool to extract the desired class from the jar file.
 * 
 * @author Yang Sun
 * 
 */
public class JarExtractor {
  private final String jarFile;

  public JarExtractor(String jarFile) {
    this.jarFile = jarFile;
  }

  /**
   * Extract the target class from the jar file.
   * 
   * @param targetClass
   * @return
   * @throws IOException
   * @throws ClassNotFoundException
   */
  public Class<?> extractClass(String targetClass) throws IOException, ClassNotFoundException {
    JarFile jar = new JarFile(jarFile);
    Enumeration<JarEntry> e = jar.entries();
    URL[] urls = { new URL("jar:file:" + jarFile + "!/") };
    URLClassLoader cl = URLClassLoader.newInstance(urls);

    while (e.hasMoreElements()) {
      JarEntry je = (JarEntry) e.nextElement();
      if (je.isDirectory() || !je.getName().endsWith(".class")) {
        continue;
      }

      String className = je.getName().substring(0, je.getName().lastIndexOf(".class"));
      className = className.replace('/', '.');
      if (className.equals(targetClass))
        return cl.loadClass(className);
    }
    return null;
  }

  // public static void main(String[] args) throws IOException, ClassNotFoundException,
  // SecurityException, NoSuchMethodException, IllegalArgumentException,
  // InstantiationException, IllegalAccessException, InvocationTargetException {
  // JarExtractor extractor = new JarExtractor("Jar/15640.jar");
  // Class<?> cl = extractor.extractClass("Examples.HelloWorld");
  // Constructor<?> constructor = cl.getConstructor();
  // constructor.newInstance();
  // }
}
