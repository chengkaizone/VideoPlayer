package org.lance.opengl;

import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import org.lance.decode.FFmpeg;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLU;

/**
 * 播放解码
 * 
 * @author lance
 * 
 */
public class PlayerSurfaceView extends GLSurfaceView {

	private PlayerRenderer playerRenderer;
	private int screenWidth;
	private int screenHeight;
	private ThreadPoolExecutor threadExecutor;
	private Thread thread;
	private int MIN_RATE = 0;
	private int MAX_RATE = 100;
	private int rateStep = 10;
	private int rate = 20;;
	private byte[] pixels;// 颜色值
	private boolean isPlay = false;

	private FFmpeg fmp;

	public PlayerSurfaceView(Context context) {
		super(context);
		playerRenderer = new PlayerRenderer();
		this.setRenderer(playerRenderer);
		this.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		fmp = FFmpeg.getInstance();

		threadExecutor = new ThreadPoolExecutor(5, 10, 30L, TimeUnit.SECONDS,
				new SynchronousQueue<Runnable>(),
				new ThreadPoolExecutor.CallerRunsPolicy());
		thread = new Thread(new Runnable() {
			@Override
			public void run() {
				while (true) {
					try {
						Thread.sleep(rate);// 这里可以控制播放的速度
						if (isPlay) {
							pixels = fmp.getNextDecodedFrame();
							requestRender();// 请求重绘
						}
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		});
		threadExecutor.execute(thread);
	}

	// 初始化播放路径
	public void initPath(String filePath) {
		fmp.openFile(filePath);
		fmp.prepareResources();
	}

	/** 开始播放 */
	public void startPlay() {
		isPlay = true;
	}

	/** 停止播放 */
	public void stopPlay() {
		isPlay = false;
	}

	/** 快进 */
	public void fastForward() {
		rate -= rateStep;
		if (rate < MIN_RATE) {
			rate = MIN_RATE;
		}
	}

	/** 快退 */
	public void fastRewind() {
		rate += rateStep;
		if (rate > MAX_RATE) {
			rate = MAX_RATE;
		}
	}

	private class PlayerRenderer implements GLSurfaceView.Renderer {

		private PlayerTexture playerTex;
		private int texId;

		@Override
		public void onDrawFrame(GL10 gl) {
			gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
			GLU.gluLookAt(gl, 0, 0, 1f, 0, 0, 0, 0, 1, 0);
			gl.glMatrixMode(GL10.GL_MODELVIEW);
			gl.glLoadIdentity();

			gl.glPushMatrix();
			gl.glTranslatef(0, 0, 0);
			gl.glRotatef(0, 0, 1, 0);
			if (playerTex != null) {
				playerTex.drawSelf(gl);
			}
			gl.glPopMatrix();
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			screenWidth = width;
			screenHeight = height;
			gl.glViewport(0, 0, width, height);
			gl.glMatrixMode(GL10.GL_PROJECTION);
			gl.glLoadIdentity();
			float ratio = (float) width / height;
			gl.glFrustumf(-ratio, ratio, -0.5f, 1f, 1f, 100f);
		}

		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			gl.glEnable(GL10.GL_DITHER);
			gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_FASTEST);
			gl.glClearColor(0, 0, 0, 0);
			gl.glEnable(GL10.GL_SMOOTH);
			gl.glEnable(GL10.GL_DEPTH_TEST);

			texId = TextureFactory.initTexture(gl, pixels);
			playerTex = new PlayerTexture(texId, screenWidth, screenHeight);
		}

	}

}
