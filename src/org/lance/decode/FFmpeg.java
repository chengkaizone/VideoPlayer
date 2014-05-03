package org.lance.decode;

import android.util.Log;

/**
 * 单例类 ffmpeg工具
 * 
 * @author lance
 * 
 */
public class FFmpeg {
	private static final String TAG = "FFmpeg";
	private static FFmpeg singleton;
	
	public static FFmpeg getInstance() {
		if (singleton == null) {
			singleton = new FFmpeg();
		}
		return singleton;
	}
	
	private FFmpeg() {
		avRegisterAll();
	}

	static {
		System.loadLibrary("ffmpeg");
		System.loadLibrary("VideoPlayer");
	}

	/** 播放指定路径的媒体文件 */
	public native String play(String filePath);

	// ffmpeg api 原始方法调用 10个方法
	/** 注册所有解码器 */
	private native void avRegisterAll();
	/** 打开输入文件 */
	private native boolean avOpenInputFile(String filePath);
	/** 初始化媒体文件 */
	private native boolean initFile(String filePath);
	/** 查找刘信息 */
	private native boolean avFindStreamInfo();
	/** 查找视频信息 */
	private native boolean findVideoStream();
	/** 查找解码器 */
	private native boolean avcodecFindDecoder();
	/** 解码 */
	private native boolean avcodecOpen();
	/** 分配帧 */
	private native void avcodecAllocFrame();
	/** 释放内存 */
	private native void avFree();
	/** 关闭解码器 */
	private native void avcodecClose();
	/** 关闭输入文件流 */
	private native void avCloseInputFile();

	// 回调给 Java 层
	public native String getCodecName();
	public native int getWidth();
	public native int getHeight();
	public native int getBitRate();
	public native boolean allocateBuffer();
	public native void setScreenSize(int sWidth, int sHeight);
	/** 返回解码图像数组 */
	public native byte[] getNextDecodedFrame();
	
	// 测试返回一个字符串
	public native static String stringFromJNI();
	// 测试返回一个int数组
	public native int[] jniIntArray();
	
	public void playFile(String filePath) {
		if(init(filePath)){
			getNextDecodedFrame();
		}
	}

	/** 初始化文件路径 1.打开媒体文件 2.查找流信息 3.查找视频流 4.查找解码器 5.用找到的解码器打开 */
	private boolean init(String filePath) {
		if (avOpenInputFile(filePath)) {
			Log.i("ff", "success open");
		} else {
			Log.i("ff", "failed open");
			return false;
		}

		if (avFindStreamInfo()) {
			Log.i("ff", "success find stream info");
		} else {
			Log.i("ff", "failed find stream info");
			return false;
		}

		if (findVideoStream()) {
			Log.i("ff", "success find stream");
		} else {
			Log.i("ff", "failed find stream");
			return false;
		}

		if (avcodecFindDecoder()) {
			Log.i("ff", "success find decoder");
		} else {
			Log.i("ff", "failed find decoder");
			return false;
		}

		if (avcodecOpen()) {
			Log.i("ff", "success codec open");
		} else {
			Log.i("ff", "failed codec open");
			return false;
		}
		Log.i("ff", getCodecName());

		return true;
	}

	/** 释放资源 */
	public void cleanUp() {
		avFree();
		avcodecClose();
		avCloseInputFile();
	}

	/** 打开文件 */
	public boolean openFile(String filePath) {
		return init(filePath);
	}

	/** 是否是媒体文件 */
	public boolean isMediaFile(String filePath) {
		avRegisterAll();
		if (avOpenInputFile(filePath)) {
			Log.i(TAG, "解码器名->"+getCodecName()+"\n"+"宽-->"+getWidth()+"\n"
					+"高-->"+getHeight()+"\n"+"比特率-->"+getBitRate());
			//allocateBuffer();
			avCloseInputFile();
			return true;
		} else {
			return false;
		}
	}

	/** 准备分配资源 */
	public void prepareResources() {
		avcodecAllocFrame();
		allocateBuffer();
	}

}
