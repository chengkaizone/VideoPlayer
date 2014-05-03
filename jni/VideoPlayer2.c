#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#include <string.h>
#include <jni.h>
#include <pthread.h>

#include <android/log.h>
//ndk log���������־�ڿ���̨��ʾ
#define LOG_TAG "System.out.c"  //__VA_ARGS__��ʾ�ɱ����
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
//ת����java���͵��ַ���
#define RETURN(msg) return (*env)->NewStringUTF(env, msg);

//ý���ļ�����Ϣ����---���ʾ���һ��ý���ļ�
AVFormatContext *pFormatCtx;
//�����������---�ͽ�����һһ��Ӧ
AVCodecContext *pCodecCtx;
//AV������
AVCodec *pCodec;
//��Ƶ��Ϣ
AVFrame *pFrame;
//�����Ǳ������Ƶ֡�İ���Ϣ--->n��AVPacket���һ��AVFrame
AVPacket packet;

//��Ƶ��
int i;
//��Ƶ��ID
int videoStreamID;
//��Ƶ��ID
int audioStreamID;
//֡��
int frameCount = 0;

int frameFinished;
//����
float aspect_ratio;

AVFrame *pFrameRGB;
int numBytes;
//���浽�����ͼ������
uint8_t *buffer;
// BE for Big Endian, LE for Little Endian
int dstFmt = PIX_FMT_RGB565;
//������Ƶ�ֱ���,ɫ�ʿռ������
struct SwsContext *img_convert_ctx;
//�������,������
int width, height, bit_rate;
//��Ļ���
int screenWidth, screenHeight;

/*****************************************************/
/* FFmpeg API jni����������cԴ�ļ��е��� */
/*****************************************************/

/** 1.ע�����б�������� */
void Java_org_lance_decode_FFmpeg_avRegisterAll(JNIEnv* env, jobject thiz) {
	av_register_all();
}

/** 2.��һ��ý���ļ� 0��ʾ��ʧ��,1��ʾ�򿪳ɹ�,ͬʱ��ʼ��ý���ļ� ! */
jboolean Java_org_lance_decode_FFmpeg_avOpenInputFile(JNIEnv* env, jobject thiz,
		jstring jfilePath) {
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	//LOGI("path---->%s",filePath);
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
		return 0;
	} else {
		return 1;
	}
}

/** close �ر�ý���ļ�����,�ͷ������ڴ�ռ� */
void Java_org_lance_decode_FFmpeg_avCloseInputFile(JNIEnv* env, jobject thiz) {
	avformat_close_input(&pFormatCtx);
}

/** ��ʼ��ý���ļ� */
jboolean Java_org_lance_decode_FFmpeg_initFile(JNIEnv *env, jobject thiz,
		jstring jfilePath) {
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	//LOGI("path---->%s",filePath);
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) == 0) {
		LOGI("success open");
	} else {
		LOGE("failed open");
		return 0;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
		LOGI("success find stream info");
	} else {
		LOGE("failed find stream info");
		return 0;
	}

	//�������е�����Ϣ---�Ƿ������Ƶ����
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamID = i;
			break;
		}
	}

	//�������е�����Ϣ---�Ƿ������Ƶ����
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStreamID = i;
			break;
		}
	}

	if (videoStreamID == -1) {
		LOGE("failed find video stream");
		//-1Ϊδ֪����Ϣ
		return 0;
	} else {
		LOGI("success find video stream");
	}

	if (audioStreamID == -1) {
		LOGE("failed find audio stream");
	} else {
		LOGI("success find audio stream  %d ", audioStreamID);
	}

	//�õ���������
	pCodecCtx = pFormatCtx->streams[videoStreamID]->codec;
	//��������ID�ҵ����ʵĽ�����
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		LOGE("failed find decoder");
		return 0;
	} else {
		LOGI("success find decoder");
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) == 0) {
		LOGI("success codec open");
	} else {
		LOGE("failed codec open");
		return 0;
	}

	LOGI("codec name--->%s", pCodec->name);
	return 1;
}

/** ��������Ϣ */
jboolean Java_org_lance_decode_FFmpeg_avFindStreamInfo(JNIEnv* env,
		jobject thiz) {
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		return 0;
	} else {
		return 1;
	}
}
/** ������Ƶ�� 0��ʾ����ʧ�� 1��ʾ�ɹ�!*/
jboolean Java_org_lance_decode_FFmpeg_findVideoStream(JNIEnv* env, jobject thiz) {
	videoStreamID = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) { //�������е�����Ϣ---�Ƿ������Ƶ����
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamID = i; //����õ���Ƶ����ID
			break;
		}
	}
	if (videoStreamID == -1) {
		return 0;
	} else {
		return 1;
	}
}
/** ���Һ��ʵĽ����� */
jboolean Java_org_lance_decode_FFmpeg_avcodecFindDecoder(JNIEnv* env,
		jobject thiz) {
	//����������,���ڷ�����ͷ�
	pCodecCtx = pFormatCtx->streams[videoStreamID]->codec;
	//��������ID�ҵ����ʵĽ�����
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		return 0;
	} else {
		return 1;
	}
}
/** �򿪽����� */
jboolean Java_org_lance_decode_FFmpeg_avcodecOpen(JNIEnv* env, jobject thiz) {
	//ʹ�ø����Ľ�������ʼ����������
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		return 0;
	} else {
		return 1;
	}
}
/** ����֡�ռ� */
void Java_org_lance_decode_FFmpeg_avcodecAllocFrame(JNIEnv* env, jobject thiz) {
	//����һ��֡�ռ�,���ݳ�ʼ��ΪĬ��ֵ---����ʧ�ܷ���NULL
	pFrame = avcodec_alloc_frame();
}
/** �ͷŷ���Ŀռ� */
void Java_org_lance_decode_FFmpeg_avFree(JNIEnv* env, jobject thiz) {
	//�ͷŷ�����ڴ�ռ�
	av_free(pFrame);
}

/** �رս����������ͷ���֮�������ڴ�ռ� */
void Java_org_lance_decode_FFmpeg_avcodecClose(JNIEnv* env, jobject thiz) {
	avcodec_close(pCodecCtx);
}

/*****************************************************/
/* �ص���javaӦ�ò�ĺ��� */
/*****************************************************/
/** ��ȡ�������� */
jstring Java_org_lance_decode_FFmpeg_getCodecName(JNIEnv* env, jobject thiz) {
	return (*env)->NewStringUTF(env, pCodec->name);
}
/** ��ȡ��Ƶ�ļ���� */
jint Java_org_lance_decode_FFmpeg_getWidth(JNIEnv* env, jobject thiz) {
	width = pCodecCtx->width;
	return width;
}
/** ��ȡ��Ƶ�ļ��߶� */
jint Java_org_lance_decode_FFmpeg_getHeight(JNIEnv* env, jobject thiz) {
	height = pCodecCtx->height;
	return height;
}
/** ��ȡ��Ƶ������ 341948 */
jint Java_org_lance_decode_FFmpeg_getBitRate(JNIEnv* env, jobject thiz) {
	bit_rate = pCodecCtx->bit_rate;
	return bit_rate;
}
/** ����֡�ṹ���� ֻ��Ҫ����һ֡ͼ�������ռ� */
jboolean Java_org_lance_decode_FFmpeg_allocateBuffer(JNIEnv* env, jobject thiz) {
	// Allocate an AVFrame structure
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL) {
		return 0;
	}
	//����Ϣ���浽debugMsg��Ȼ���ӡ����־
	LOGI("screen: width=%d height=%d", screenWidth, screenHeight);
	LOGI("video: width=%d height=%d", pCodecCtx->width, pCodecCtx->height);
	// ������Ļ��ߺ�ͼƬ����ȷ����������С
	numBytes = avpicture_get_size(dstFmt, screenWidth, screenHeight);
	//numBytes=avpicture_get_size(dstFmt, pCodecCtx->width,pCodecCtx->height);
	//��̬����ָ����С���ڴ�ռ䱣�浽������
	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

	//�����ʵ��Ļ��沿�ֵ�rgbͼ��֡�� AVFrame��AVPicture�ĳ���---�������ͼ����Ϣ
	avpicture_fill((AVPicture *) pFrameRGB, buffer, dstFmt, screenWidth,
			screenHeight);

	return 1;
}
/** ������Ļ�ߴ� */
void Java_org_lance_decode_FFmpeg_setScreenSize(JNIEnv* env, jobject thiz,
		int sWidth, int sHeight) {
	screenWidth = sWidth;
	screenHeight = sHeight;
}

/* ����һ��������һ֡ */
jbyteArray Java_org_lance_decode_FFmpeg_getNextDecodedFrame(JNIEnv* env,
		jobject thiz) {
	//����ǰ���ͷ���һ֡����İ���Ϣ
	av_free_packet(&packet);
	//�����������Ч����֮�����Ч֡�������������Ϣ���롣������Ϣ������packet��
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoStreamID) {
			//ȡ����Ƶ���Ĳ���---�����������Ƶ֡ת����ͼƬ---���ֽ������ڵ���AVPacket֧�ֶ��֡,�����Ľ������͵�һ֡���롣
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			LOGI("length->%d video stream:%d", frameFinished, packet.stream_index);
			if (frameFinished) {
				img_convert_ctx = sws_getCachedContext(img_convert_ctx,
						pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
						screenWidth, screenHeight, dstFmt, SWS_BICUBIC, NULL,
						NULL, NULL);
				//img_convert_ctx = sws_getContext(pCodecCtx->width,
				//		pCodecCtx->height, pCodecCtx->pix_fmt, screenWidth,
				//	screenHeight, dstFmt, SWS_BICUBIC, NULL, NULL, NULL);
				//��ԴƬ���ŵ�Ŀ��Ƭ��
				sws_scale(img_convert_ctx,
						(const uint8_t* const *) pFrame->data, pFrame->linesize,
						0, pCodecCtx->height, pFrameRGB->data,
						pFrameRGB->linesize);
				++frameCount;
				/* uint8_t == unsigned 8 bits == jboolean ����ͼ���������� */
				jbyteArray nativePixels = (*env)->NewByteArray(env, numBytes);
				//ʹ�û���Ϊͼ���������鸳ͼ������
				(*env)->SetByteArrayRegion(env, nativePixels, 0, numBytes,
						buffer);
				return nativePixels;
			}
		}
		av_free_packet(&packet);
	}
	return NULL;
}

/* ����ָ��·������Ƶ �ú���û�������߳���ʹ�� */
jstring Java_org_lance_decode_FFmpeg_play(JNIEnv* env, jobject thiz,
		jstring jfilePath) {
	//����ͷ��ز���·��
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	RETURN(filePath);
	AVFormatContext *pFormatCtx;
	int i, videoStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVPacket packet;
	int frameFinished;
	float aspect_ratio;
	struct SwsContext *img_convert_ctx;

	/* FFmpeg */
	av_register_all();
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
		RETURN("failed avformat_open_input ");
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		RETURN("failed av_find_stream_info");
	}

	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1) {
		RETURN("failed videostream == -1");
	}

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		RETURN("Unsupported codec!");
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		RETURN("failed codec_open");
	}

	pFrame = avcodec_alloc_frame();

	/* FFmpeg */

	LOGI("decodec :%s", pCodec->name);
	LOGI("Getting into stream decode:");

	/* video stream */
	//����ǰ���ͷ���һ֡����İ���Ϣ
//		av_free_packet(&packet);
//		//�����������Ч����֮�����Ч֡�������������Ϣ���롣������Ϣ������packet��
//		while (av_read_frame(pFormatCtx, &packet) >= 0) {
//			if (packet.stream_index == videoStreamID) {
//				//ȡ����Ƶ���Ĳ���---�����������Ƶ֡ת����ͼƬ---���ֽ������ڵ���AVPacket֧�ֶ��֡,�����Ľ������͵�һ֡���롣
//				avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
//				LOGI("length->%d video stream:%d", frameFinished, packet.stream_index);
//				if (frameFinished) {
//					img_convert_ctx = sws_getCachedContext(img_convert_ctx,
//							pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
//							screenWidth, screenHeight, dstFmt, SWS_BICUBIC, NULL,
//							NULL, NULL);
//					//img_convert_ctx = sws_getContext(pCodecCtx->width,
//					//		pCodecCtx->height, pCodecCtx->pix_fmt, screenWidth,
//					//	screenHeight, dstFmt, SWS_BICUBIC, NULL, NULL, NULL);
//					//��ԴƬ���ŵ�Ŀ��Ƭ��
//					sws_scale(img_convert_ctx,
//							(const uint8_t* const *) pFrame->data, pFrame->linesize,
//							0, pCodecCtx->height, pFrameRGB->data,
//							pFrameRGB->linesize);
//					++frameCount;
//					/* uint8_t == unsigned 8 bits == jboolean ����ͼ���������� */
//					jbyteArray nativePixels = (*env)->NewByteArray(env, numBytes);
//					//ʹ�û���Ϊͼ���������鸳ͼ������
//					(*env)->SetByteArrayRegion(env, nativePixels, 0, numBytes,
//							buffer);
//					return nativePixels;
//				}
//			}
//			av_free_packet(&packet);
//		}

	/* video stream */
	av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return (*env)->NewStringUTF(env, "end of main");
}

// TODO =============================���ӹ�����======================

/** ����JNI */
jstring Java_org_lance_decode_FFmpeg_stringFromJNI(JNIEnv* env, jobject thiz) {
	return (*env)->NewStringUTF(env, "Helloworld from FFmpeg!");
}

/** ���Է���һ������ */
jintArray Java_org_lance_decode_FFmpeg_jniIntArray(JNIEnv* env, jobject thiz) {
	LOGI("test return array~");
	int nativeIntArray[2];
	nativeIntArray[0] = 330;
	nativeIntArray[1] = 2;
	LOGI("0 %d", nativeIntArray[0]);
	LOGI("1 %d", nativeIntArray[1]);

	jintArray nativeReturn = (*env)->NewIntArray(env, 2);
	(*env)->SetIntArrayRegion(env, nativeReturn, 0, 2, nativeIntArray);
	return nativeReturn;
}

/** ��java�������ַ���ת����c���Ե��ַ��� �������÷������ */
char* Jstring2CStr(JNIEnv* env, jstring jstr) {
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "GB2312");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes",
			"(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray) (*env)->CallObjectMethod(env, jstr, mid,
			strencode); // String .getByte("GB2312");
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) malloc(alen + 1); //"\0"
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	//rtn�Ƕ�̬�ڴ����---��baָ������ݿ�����rtn��,���Ժ�����Ҫ�ͷŵ���baָ��
	(*env)->ReleaseByteArrayElements(env, barr, rtn, 0); //
	return rtn;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL; //����JNI Env
	jint result = JNI_ERR;
	/*JavaVM::GetEnv ԭ��Ϊ jint (*GetEnv)(JavaVM*, void**, jint);
	 * GetEnv()�������ص�  Jni ������ÿ���߳���˵�ǲ�ͬ�ģ�
	 *  ����Dalvik�����ͨ����Multi-threading�ġ�ÿһ���̵߳���JNI_OnLoad()ʱ��
	 *  ���õ�JNI Env�ǲ�ͬ�ģ�������Ǳ�����ÿ�ν��뺯��ʱ��Ҫͨ��vm->GetEnv���»�ȡ
	 *
	 */
	//�õ�JNI Env
//	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
//		LOGE("GetEnv failed!");
//		return result;
//	}
//	LOGI("loading...");
//
//	/*��ʼע��
//	 * ���������JNI env
//	 * ���������кܶ�class��ֻ��register_android_media_FFMpegPlayerAndroid(env)Ϊ����
//	 */
//
//#ifdef BUILD_WITH_CONVERTOR
//	if(register_android_media_FFMpeg(env) != JNI_OK) {
//		__android_log_print(ANDROID_LOG_ERROR, TAG, "can't load android_media_FFMpeg");
//		goto end;
//	}
//#endif
//	if (register_android_media_FFMpegAVFormatContext(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVFormatContext");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVCodecContext(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVCodecContext");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVRational(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVRational");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVInputFormat(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVInputFormat");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegUtils(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegUtils");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVFrame(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVFrame");
//		goto end;
//	}
//
//#ifdef BUILD_WITH_PLAYER
//	if(register_android_media_FFMpegPlayerAndroid(env) != JNI_OK) { //����----��com_media_ffmpeg_FFMpegPlayer.cpp�ļ�
//		LOGE("can't load android_media_FFMpegPlayerAndroid");
//		goto end;
//	}
//#endif
//	LOGI("loaded!");

	result = JNI_VERSION_1_4;

	return result;
}
