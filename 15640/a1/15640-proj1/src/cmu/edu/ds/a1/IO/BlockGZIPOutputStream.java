package cmu.edu.ds.a1.IO;

import java.io.IOException;
import java.io.OutputStream;
import java.util.zip.GZIPOutputStream;

/**
 * This serves as the utility class to help serialize the object
 * 
 * @author Yang Sun, Yuan Gu
 * 
 */
public class BlockGZIPOutputStream extends GZIPOutputStream {

	@Override
	public void write(byte[] b) throws IOException {
		out.write(b);
	}

	public BlockGZIPOutputStream(OutputStream out) throws IOException {
		super(out);
	}
}
