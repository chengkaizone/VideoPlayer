package org.lance.decode;

import android.util.Log;

/**
 * ������ ffmpeg����
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

	/** ����ָ��·����ý���ļ� */
	public native String play(String filePath);

	// ffmpeg api ԭʼ�������� 10������
	/** ע�����н����� */
	private native void avRegisterAll();
	/** �������ļ� */
	private native boolean avOpenInputFile(String filePath);
	/** ��ʼ��ý���ļ� */
	private native boolean initFile(String filePath);
	/** ��������Ϣ */
	private native boolean avFindStreamInfo();
	/** ������Ƶ��Ϣ */
	private native boolean findVideoStream();
	/** ���ҽ����� */
	private native boolean avcodecFindDecoder();
	/** ���� */
	private native boolean avcodecOpen();
	/** ����֡ */
	private native void avcodecAllocFrame();
	/** �ͷ��ڴ� */
	private native void avFree();
	/** �رս����� */
	private native void avcodecClose();
	/** �ر������ļ��� */
	private native void avCloseInputFile();

	// �ص��� Java ��
	public native String getCodecName();
	public native int getWidth();
	public native int getHeight();
	public native int getBitRate();
	public native boolean allocateBuffer();
	public native void setScreenSize(int sWidth, int sHeight);
	/** ���ؽ���ͼ������ */
	public native byte[] getNextDecodedFrame();
	
	// ���Է���һ���ַ���
	public native static String stringFromJNI();
	// ���Է���һ��int����
	public native int[] jniIntArray();
	
	public void playFile(String filePath) {
		if(init(filePath)){
			getNextDecodedFrame();
		}
	}

	/** ��ʼ���ļ�·�� 1.��ý���ļ� 2.��������Ϣ 3.������Ƶ�� 4.���ҽ����� 5.���ҵ��Ľ������� */
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

	/** �ͷ���Դ */
	public void cleanUp() {
		avFree();
		avcodecClose();
		avCloseInputFile();
	}

	/** ���ļ� */
	public boolean openFile(String filePath) {
		return init(filePath);
	}

	/** �Ƿ���ý���ļ� */
	public boolean isMediaFile(String filePath) {
		avRegisterAll();
		if (avOpenInputFile(filePath)) {
			Log.i(TAG, "��������->"+getCodecName()+"\n"+"��-->"+getWidth()+"\n"
					+"��-->"+getHeight()+"\n"+"������-->"+getBitRate());
			//allocateBuffer();
			avCloseInputFile();
			return true;
		} else {
			return false;
		}
	}

	/** ׼��������Դ */
	public void prepareResources() {
		avcodecAllocFrame();
		allocateBuffer();
	}

}
