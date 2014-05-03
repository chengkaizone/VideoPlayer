package org.lance.opengl;

import javax.microedition.khronos.opengles.GL10;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLUtils;

/**
 * ������
 * 
 * @author lance
 * 
 */
public class TextureFactory {

	/** ͨ����ɫֵ�������� */
	public static int initTexture(GL10 gl, byte[] pixels) {
		int[] trr = new int[1];
		gl.glGenTextures(1, trr, 0);
		int curTexId = trr[0];
		gl.glBindTexture(GL10.GL_TEXTURE_2D, curTexId);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER,
				GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER,
				GL10.GL_LINEAR);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S,
				GL10.GL_REPEAT);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T,
				GL10.GL_REPEAT);
		try {
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inJustDecodeBounds = true;
			Bitmap bitmap = BitmapFactory.decodeByteArray(pixels, 0,
					pixels.length, options);
			options.inSampleSize = 1;
			options.inJustDecodeBounds = false;
			options.inPurgeable = true;
			bitmap = BitmapFactory.decodeByteArray(pixels, 0, pixels.length,
					options);
			GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
			bitmap.recycle();
		} catch (Exception e) {
			e.printStackTrace();
		} finally {

		}
		return curTexId;
	}

	/** ͨ��λͼ�������� */
	public static int initTexture(GL10 gl, Bitmap bitmap) {
		int[] trr = new int[1];
		gl.glGenTextures(1, trr, 0);// ��������������
		int curTexId = trr[0];
		gl.glBindTexture(GL10.GL_TEXTURE_2D, curTexId);// ��gl�а����������
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER,
				GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER,
				GL10.GL_LINEAR);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_S,
				GL10.GL_REPEAT);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_WRAP_T,
				GL10.GL_REPEAT);
		try {
			GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, bitmap, 0);
			bitmap.recycle();
		} catch (Exception e) {
			e.printStackTrace();
		} finally {

		}
		return curTexId;
	}
}
