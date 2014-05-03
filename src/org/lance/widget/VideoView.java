package org.lance.widget;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class VideoView extends View {
	public VideoView(Context context, AttributeSet attrs) {
		super(context, attrs);
		if (DBG)
			Log.i(TAG, "\t + RemoteView construction");
		setFocusable(true);
		if (DBG)
			Log.i(TAG, "\t - RemoteView construction");
	}

	public final int getViewWidth() {
		return mWidth;
	}

	public final int getViewHeight() {
		return mHeight;
	}

	public void setSourceSize(int width, int height) {
		if ((mWidth != width) || (mHeight != height)) {
			if (null != mBitmap) {
				mBitmap.recycle();
				mBitmap = null;
			}

			if (DBG)
				Log.i(TAG, "\t \t create initial bitmap: " + mWidth + " * "
						+ mHeight);
			mBitmap = Bitmap.createBitmap(width, height, Config.RGB_565);
		}

		mWidth = width;
		mHeight = height;

		if (null == mSrcRect) {
			mSrcRect = new Rect();
		}
		mSrcRect.set(0, 0, width, height);
	}

	public void setUIHandler(Handler handler) {
		mHandler = handler;
	}

	public final void setFreshingFlag(boolean mIsFreshing) {
		if (DBG)
			Log.i(TAG, "\t + setFreshingFlag(" + mIsFreshing + ")");
		Message msg = new Message();
		// msg.what = VideoChatActivity.EVENT_REMOTE_VIEW_FRESHING;
		msg.arg1 = mIsFreshing ? 1 : 0;
		mHandler.sendMessage(msg);
		if (DBG)
			Log.i(TAG, "\t - setFreshingFlag(" + mIsFreshing + ")");
		return;
	}

	public void renderData(byte[] pixelData) {
		if (DBG)
			Log.i(TAG, "\t renderData()");
		if (null != pixelData) {
			ByteBuffer buf = ByteBuffer.wrap(pixelData);
			buf.position(0);
			if (null != mBitmap) {
				mBitmap.copyPixelsFromBuffer(buf);
			}

			postInvalidate();
		}
	}

	public void reset() {
		if (mBitmap != null) {
			// mBitmap.recycle();
			mBitmap.eraseColor(Color.BLACK);
			// mBitmap = null;
			postInvalidate();
		}
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);

		if (null == mDestRect) {
			mDestRect = new Rect();
			mDestRect.set(0, 0, this.getWidth(), this.getHeight());
		}

		if (null != mBitmap) {
			// canvas.drawBitmap(mBitmap, 0, 0, null);
			canvas.drawBitmap(mBitmap, mSrcRect, mDestRect, null);
		} else {
			canvas.drawColor(Color.BLACK);
		}
	}

	public void doCupture(String filePath) {
		if (null != mBitmap) {

			try {
				File fileDirs = new File(filePath.substring(0,
						filePath.lastIndexOf("/")));
				File file = new File(filePath);
				FileOutputStream fOut = null;

				if (!fileDirs.exists() || !fileDirs.isDirectory()) {
					fileDirs.mkdirs();
					fileDirs = null;
				}

				file.createNewFile();

				fOut = new FileOutputStream(file);
				mBitmap.compress(Bitmap.CompressFormat.JPEG, 100, fOut);
				fOut.flush();
				fOut.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
		}
	}

	private Handler mHandler = null;

	private int mWidth = 0;
	private int mHeight = 0;
	private Bitmap mBitmap = null;

	private Rect mSrcRect = null;
	private Rect mDestRect = null;

	private static final String TAG = "VideoView";
	private static final boolean DBG = false;
}
