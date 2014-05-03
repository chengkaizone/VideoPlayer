package org.lance.activity;

import java.nio.ByteBuffer;

import org.lance.decode.FFmpeg;
import org.lance.other.PlayerActivity;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;

/**
 * 开始播放视频
 * 
 * @author lance
 * 
 */
public class VideoPlayerActivity extends Activity {

	private PlayerView playerView;

	public void onCreate(android.os.Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		DisplayMetrics dm = new DisplayMetrics();
		this.getWindowManager().getDefaultDisplay().getMetrics(dm);
		playerView = new PlayerView(this, dm.widthPixels, dm.heightPixels);
		setContentView(playerView);

		playerView.setContext(this);

		String path = getIntent().getStringExtra("PATH");
		System.out.println("文件路径--->" + path);
		if (path != null && !path.equals("")) {
			// playerView.play(path);
		}
	}

	class PlayerView extends View implements Runnable {

		private Bitmap bitmap;
		private Paint mPaint;
		private FFmpeg ff;
		private int width;
		private int height;
		private byte[] nativePixels;
		private ByteBuffer buffer;
		private int displayWidth;
		private int displayHeight;
		private Activity mContext;

		public PlayerView(Context context, int DisplayWidth, int DisplayHeight) {
			super(context);
			ff = FFmpeg.getInstance();

			this.displayWidth = DisplayWidth;
			this.displayHeight = DisplayHeight;
			mPaint = new Paint();
		}

		public void setContext(Activity ctx) {
			this.mContext = ctx;
		}

		public void play(String filePath) {
			Log.i("playerView", "to play: " + filePath);

			ff.openFile(filePath);
			width = ff.getWidth();
			height = ff.getHeight();

			/* adjust orientation */
			if (width > height) {
				mContext.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
				ff.setScreenSize(displayHeight, displayWidth);
			}
			ff.setScreenSize(displayWidth, displayHeight);

			ff.prepareResources();

			bitmap = Bitmap.createBitmap(displayWidth, displayHeight,
					Bitmap.Config.RGB_565);

			/* 开启线程 */
			new Thread(this).start();
		}

		public void onDraw(Canvas canvas) {
			super.onDraw(canvas);
			// canvas.drawColor(Color.GREEN);
			// nativePixels = ff.getNextDecodedFrame();
			// if(nativePixels != null) {
			// buffer = ByteBuffer.wrap(nativePixels);
			// bitmap.copyPixelsFromBuffer(buffer);
			// }
			//
			// canvas.drawBitmap(bitmap, 0, 0, mPaint);
		}

		public void run() {
			while (!Thread.currentThread().isInterrupted()) {
				try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					Thread.currentThread().interrupt();
				}

				// 使用 postInvalidate 可以直接在线程中更新界面
				postInvalidate();
			}
		}
	}
}
