package org.lance.opengl;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.opengles.GL10;

/**
 * ²¥·ÅÆ÷ÎÆÀí
 * 
 * @author lance
 * 
 */
public class PlayerTexture {
	private FloatBuffer vertexBuffer;
	private FloatBuffer textureBuffer;
	private final float UNIT_SIZE = 1f;
	private final int vCount = 6;
	private int texId;

	public PlayerTexture(int texId, float width, float height) {
		this.texId = texId;
		float[] vertices = { -width * UNIT_SIZE, height * UNIT_SIZE, 0,
				-width * UNIT_SIZE, -height * UNIT_SIZE, 0, width * UNIT_SIZE,
				-height * UNIT_SIZE, 0,

				width * UNIT_SIZE, -height * UNIT_SIZE, 0, width * UNIT_SIZE,
				height * UNIT_SIZE, 0, -width * UNIT_SIZE, height * UNIT_SIZE,
				0, };
		ByteBuffer vbb = ByteBuffer.allocateDirect(vCount * 3 * 4);
		vbb.order(ByteOrder.nativeOrder());
		vertexBuffer = vbb.asFloatBuffer();
		vertexBuffer.put(vertices);
		vertexBuffer.position(0);

		float[] texST = { 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0 };
		ByteBuffer tbb = ByteBuffer.allocateDirect(vCount * 2 * 4);
		tbb.order(ByteOrder.nativeOrder());
		textureBuffer = tbb.asFloatBuffer();
		textureBuffer.put(texST);
		textureBuffer.position(0);
	}

	public void drawSelf(GL10 gl) {
		gl.glEnable(GL10.GL_TEXTURE_2D);
		gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glVertexPointer(3, GL10.GL_FLOAT, 0, vertexBuffer);
		gl.glEnableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
		gl.glTexCoordPointer(2, GL10.GL_FLOAT, 0, textureBuffer);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, texId);
		gl.glDrawArrays(GL10.GL_TRIANGLES, 0, vCount);
		gl.glDisableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
		gl.glDisable(GL10.GL_TEXTURE_2D);
	}
}
